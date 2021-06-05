#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<errno.h>
#include<mqueue.h>
#include<pthread.h>

#include"definition.h"
#include"function.h"

static struct sigevent* notification;


static void CreateMQ(mqd_t *msg_q)
{
    struct mq_attr *attr=(struct mq_attr*)malloc(sizeof(struct mq_attr));
    attr->mq_maxmsg=MSG_QUEUE_MAXMSG;
    attr->mq_msgsize=MSG_QUEUE_MSGSIZE;
    attr->mq_curmsgs=0;
    attr->mq_flags=O_NONBLOCK;
    
    if((*msg_q=mq_open(MSG_QUEUE_ADDR,O_RDWR|O_CREAT,0664,attr))==-1)
        printf("ERROR OPEN: %s\n",strerror(errno));

    if(mq_setattr(*msg_q,attr,NULL)==-1)
        printf("ERROR SETATTR: %s\n",strerror(errno));
}

static void MQThread(union sigval sv)
{
    printf("This is the Notify Function\n");

    mqd_t msg_q=*(mqd_t*)sv.sival_ptr;
    mq_notify(msg_q,notification);

    char *buff=(char *)malloc(MSG_QUEUE_MSGSIZE*sizeof(char));
    ssize_t msgnum;

    while((msgnum=mq_receive(msg_q,buff,MSG_QUEUE_MSGSIZE,NULL))>=0)
    {
        printf("Read %d bytes\n",msgnum);
        printf("%s",buff);
        //在本函数中处理Worker发送数据
    }

    free(buff);
    pthread_exit(NULL);
}


void RegisterNofify(mqd_t *msg_q)
{
    notification=(struct sigevent*)malloc(sizeof(struct sigevent));
    notification->sigev_notify=SIGEV_THREAD;
    notification->sigev_notify_function=MQThread;
    notification->sigev_value.sival_ptr=msg_q;
    mq_notify(*msg_q,notification);
}

void secureINTR(int sig)
{
    printf("Admin Caught INTR\n");
    if(sig==SIGINT)
    {
        mq_unlink(MSG_QUEUE_ADDR);
    }
    exit(0);
}

int main()
{

    struct sigaction *act=(struct sigaction*)malloc(sizeof(struct sigaction));
    act->sa_handler=secureINTR;

    if(sigaction(SIGINT,act,NULL)==-1)
    {
        pError(errno);
        exit(0);
    }


    mqd_t msg_q;
    CreateMQ(&msg_q);
    RegisterNofify(&msg_q);
    

    
    
    while (true)
    {
        pause();
    }
    
    mq_unlink(MSG_QUEUE_ADDR);    
    exit(0);

}