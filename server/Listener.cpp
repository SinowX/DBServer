#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<errno.h>

#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/wait.h>
#include"function.h"
#include"definition.h"
#include<signal.h>

int curFd=-1;
int listenFd=-1;

void secureINTR(int sig)
{
    printf("Listener Caught INTR\n");
    if(sig==SIGINT)
    {
        if(curFd>=0)
            close(curFd);
        if(listenFd>=0)
            close(listenFd);
    }
    exit(0);
}


int main(int argc, char *argv[])
{
    struct sigaction *act=(struct sigaction*)malloc(sizeof(struct sigaction));
    act->sa_handler=secureINTR;

    if(sigaction(SIGINT,act,NULL)==-1)
    {
        pError(errno);
        exit(0);
    }


    int listen_fd, accept_fd;
    struct sockaddr_in server_addr,client_addr;
    char buff[4096];



    if((listen_fd = socket(AF_INET,SOCK_STREAM,0))==-1)
        pError(errno);
    
    listenFd=listen_fd;

    server_addr.sin_family=AF_INET;
    server_addr.sin_port=htons(LISTEN_PORT);
    server_addr.sin_addr.s_addr=inet_addr(LISTEN_ADDR);
    
    if(bind(listen_fd,(struct sockaddr*)&server_addr,sizeof(server_addr))==-1)
    {
        pError(errno);
        exit(-1);
    }
        

    if(listen(listen_fd,10)==-1)
        pError(errno);
    else
        printf("Server Socket Listening...\n");

    
    while(true)
    {
        if((accept_fd = accept(listen_fd,NULL,NULL))==-1)
        {
            pError(errno);
            continue;
        }else{
            printf("Connection Built :%d\n",accept_fd);
            
            switch (fork())
            {
            case -1:
            {
                pError(errno);
                break;
            }
            case 0:
            {
                printf("This is Child Process\n");
                char *str=(char*)malloc(10*sizeof(char));
                if(sprintf(str,"%d",accept_fd)==-1)
                    pError(errno);
                printf("%s\n",str);
                char *arguments[]={str,NULL};
                if(execve("Worker",arguments,NULL)==-1)
                    printf("ERROR EXECVE: %s\n",strerror(errno));
                break;
            }
            default:
            {
                if(close(accept_fd)==-1)
                    printf("ERROR CLOSE FD: %s\n",strerror(errno));
                else
                    printf("CLOSE FD SUCCESS\n");
                break;
            }
            }
        }
    }
}