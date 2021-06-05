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
#include"client_def.h"

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
    if((connect_fd = socket(AF_INET,SOCK_STREAM,0))==-1)
    {
        printf("ERROR: %s\n",strerror(errno));
        exit(-1);
    }    
    Curfd=connect_fd;

    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port=htons(SERVER_PORT);
    server_addr.sin_addr.s_addr=inet_addr(SERVER_ADDR);
    
    if(connect(connect_fd,(struct sockaddr*)&server_addr,sizeof(server_addr))==-1)
    {
        printf("ERROR CONNECT: %s\n",strerror(errno));
        exit(-1);
    }
    else{
        printf("CONNECT SUCCESS\n");
    }

    return connect_fd;
}


int main()
{
    struct sigaction act;
    registerSigHandler(&act);

    int connect_fd=connectServer();

    char *buff=(char *)malloc(BUF_SZ*sizeof(char));
    while (true)
    {
        fgets(buff,BUF_SZ-1,stdin);// BUF_SZ-1 is for '\0'
        write(connect_fd,buff,BUF_SZ);
    }
    
    

    // char opt;
    // while(true)
    // {
    //     fprintf(stdout,"Input to Switch Pack:\n");
    //     fprintf(stdout," 1 to Add\n");
    //     fprintf(stdout," 2 to Del\n");
    //     fprintf(stdout," 3 to Mod\n");
    //     fprintf(stdout," 4 to Sel\n");
    //     fprintf(stdout," 5 to Start Session\n");
    //     fprintf(stdout," 6 to End Session\n");
    //     scanf("%c",&opt);
    //     switch (opt)
    //     {
    //     case '1':
    //         break;
    //     case '2':
    //         break;
    //     case '3':
    //         break;
    //     case '4':
    //         break;
    //     case '5':
    //         break;
    //     case '6':
    //         break;
    //     default:
    //         fprintf(stdout,"Invalid Input");
    //         break;
    //     }
    // }



}