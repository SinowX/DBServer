#include<fcntl.h>
#include<sys/stat.h>
#include<mqueue.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<errno.h>

int main()
{
    struct mq_attr *attr=(struct mq_attr*)malloc(sizeof(struct mq_attr));

    mqd_t msg_q;
    if((msg_q=mq_open("/fuck",O_RDWR | O_CREAT, 0666,NULL))==-1)
        printf("ERROR: %s\n",strerror(errno));
    
    char msg[]="This is the Hello World For POSIX M_QUEUE";
    mq_send(msg_q,msg,sizeof(msg),1);
    

    if((mq_send(msg_q,"asiudhui",sizeof(msg),1))==-1)
        printf("ERROR: %s\n",strerror(errno));
    
    
    // mq_send(msg_q,"wehjuiw",sizeof(msg),1);
    

    printf("%d\n",sizeof(msg));

    mq_getattr(msg_q,attr);

    // char *rev=(char *)malloc(50*sizeof(char));
    // mq_receive(msg_q,rev,attr->mq_msgsize,NULL);
    // printf("%s",rev);


    printf("%d\n",attr->mq_flags);
    printf("%d\n",attr->mq_curmsgs);
    printf("%d\n",attr->mq_maxmsg);
    printf("%d\n",attr->mq_msgsize);
    // sleep(30);
    // mq_close(msg_q);
    // mq_unlink("/fuck");
    exit(0);
}