#include<stdio.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<errno.h>
#include<string.h>
#include<unistd.h>
#include<sys/wait.h>

void pErr()
{
    printf("ERROR: %s\n",strerror(errno));
    exit(-1);
}


int main(int argc,char *argv[])
{

    char *buffer=(char *)malloc(256*sizeof(char));
    char pathname[]="text.txt";
    size_t count=24;
    ssize_t numread=0;
    // sleep(60);
    // printf("How Many to read?\n");
    // scanf("%d",&count);
    printf("%d to be Read\n",count);

    int fd=10;
    if((fd=open(pathname,O_RDONLY|O_CREAT,0644))==-1)
        pErr();
    else
        printf("Open Success: %d\n",fd);


    if((numread=read(fd,buffer,count))==-1)
        pErr();
    else
        printf("%s\n",buffer);


    switch (fork())
    {
    case -1:
        pErr();
        break;
    case 0:
        printf("Child Process\n");

        char *str=(char*)malloc(10*sizeof(char));
        sprintf(str,"%d",fd);
        char *str2=(char*)malloc(10*sizeof(char));
        strcpy(str2,"asdaseaw");


        char *arguments[]={argv[0],str,str2,NULL};
        if(execve("test",arguments,NULL)==-1)
            pErr();
        
        break;
    default:
        close(fd);
        wait(NULL);
        break;
    }


    exit(0);
}