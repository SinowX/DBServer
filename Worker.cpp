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

int curFd=-1;

void SecureINTR(int sig)
{
    printf("Worker Caught INTR\n");
    if(sig==SIGINT&&curFd>=0)
        close(curFd);
    exit(0);
}

void RegisterSig()
{
    struct sigaction *act=(struct sigaction*)malloc(sizeof(struct sigaction));
    act->sa_handler=SecureINTR;

    if(sigaction(SIGINT,act,NULL)==-1)
    {
        pError(errno);
        exit(0);
    }
}

int main(int argc, char *argv[])
{
    printf("This is Workder\n");
    int fd;
    if((fd=atoi(argv[argc-1]))==0&&strlen(argv[argc-1])!=1)
        exit(-1); // Exec Not as a Child Process
        
    curFd=fd;
    RegisterSig();

    

}