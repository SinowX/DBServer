#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<errno.h>

#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include"function.h"
#include"definition.h"
#include<mqueue.h>
#include<fcntl.h>


int curFd=-1;
mqd_t msg_q;

void SecureINTR(int sig)
{
    printf("Worker Caught INTR\n");
    if(sig==SIGINT&&curFd>=0)
    {
        close(msg_q);
        close(curFd);
    }
        
    exit(0);
}

void RegisterSig()
{
    struct sigaction *act=(struct sigaction*)malloc(sizeof(struct sigaction));
    act->sa_handler=SecureINTR;

    if(sigaction(SIGINT,act,NULL)==-1)
    {
        printf("ERROR REGISTER SIGACTION: %s\n",strerror(errno));
        exit(0);
    }
}


mqd_t MQRegister()
{
    struct mq_attr *attr=(struct mq_attr*)malloc(sizeof(struct mq_attr));
    mqd_t msg_q;
    msg_q=mq_open(MSG_QUEUE_ADDR,O_WRONLY);
    free(attr);
    return msg_q;
}



int main(int argc, char *argv[])
{
    printf("This is Workder\n");
    int fd;
    if((fd=atoi(argv[argc-1]))==0&&strlen(argv[argc-1])!=1)
        exit(-1); // Exec Not as a Child Process
        
    curFd=fd;

    RegisterSig();

    if((msg_q=MQRegister())==-1)
    {
        printf("ERROR: %s\n",strerror(errno));
        exit(-1);
    }



    char *buff=(char *)malloc(BUF_SZ*sizeof(char));

    while(true)
    {
        int stat=read(curFd,buff,BUF_SZ);
        if(stat==0||stat==-1)
        {
            exit(stat);
        }else{
            mq_send(msg_q,buff,BUF_SZ,1);
            // printf("%s",buff);
        }
    }
}