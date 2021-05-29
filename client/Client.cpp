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
#include<signal.h>
// #include"client_def.h"

int Curfd=-1;

void secureINTR(int sig)
{
    printf("Client Caught INTR\n");
    if(Curfd>=0)
        close(Curfd);
    exit(0);
}

void registerSigHandler(struct sigaction *act)
{
    act->sa_handler=secureINTR;

    if(sigaction(SIGINT,act,NULL)==-1)
    {
        printf("ERROR SIGACTION: %s\n",strerror(errno));
        exit(0);
    }
}

int connectServer()
{
    int connect_fd;
    struct sockaddr_in server_addr;
    if((connect_fd = socket(AF_INET,SOCK_STREAM,0))!=-1)
    {
        printf("ERROR: %s\n",strerror(errno));
        exit(-1);
    }    
    
    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port=htons(SERVER_PORT);
    server_addr.sin_addr.s_addr=inet_addr("127.0.0.1");
    

    if((connect_fd = socket(AF_INET,SOCK_STREAM,0))!=-1)
    {
        Curfd=connect_fd;
        printf("CONNECT SUCCESS\n");
    }
    else{
        printf("ERROR CONNECT: %s\n",strerror(errno));
        exit(-1);
    }






    return connect_fd;
}


int main()
{
    struct sigaction act;
    registerSigHandler(&act);

    int connect_fd=connectServer();


}