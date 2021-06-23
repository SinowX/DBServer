#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<errno.h>
#include<mqueue.h>
#include<pthread.h>
#include<sys/stat.h>
#include<fcntl.h>
#include"../unidef/definition.h"
#include"function.h"
#include"../database/header/rt_class.h"


static struct sigevent* notification;

static bool isLocked=false;
static uint occupy_worker=0;

DBMGR dbmgr;

static void CreateMQ(mqd_t *msg_q)
{
    printf("Now Creating MQ\n");
    struct mq_attr *attr=(struct mq_attr*)malloc(sizeof(struct mq_attr));
    attr->mq_maxmsg=MSG_QUEUE_MAXMSG;
    attr->mq_msgsize=MSG_QUEUE_MSGSIZE;
    attr->mq_curmsgs=0;
    attr->mq_flags=O_NONBLOCK;
    
    if((*msg_q=mq_open(MSG_QUEUE_ADDR,O_RDWR|O_CREAT,0664,attr))==-1)
        printf("ERROR OPEN: %s\n",strerror(errno));

    if(mq_setattr(*msg_q,attr,NULL)==-1)
        printf("ERROR SETATTR: %s\n",strerror(errno));
    printf("Createing Finish\n");
}

static void MQThread(union sigval sv)
{
    printf("This is the Notify Function\n");

    mqd_t msg_q=*(mqd_t*)sv.sival_ptr;
    mq_notify(msg_q,notification);

    char *buff=(char *)malloc(MSG_QUEUE_MSGSIZE*sizeof(char));
    ssize_t msgnum;
    CMD::UniCMD uni_cmd;

    while((msgnum=mq_receive(msg_q,buff,MSG_QUEUE_MSGSIZE,NULL))>=0)
    {
        printf("Read %d bytes\n",msgnum);
        if(msgnum!=sizeof(CMD::UniCMD))
            printf("Message Broken");
        // printf("%s",buff);
        memcpy(&uni_cmd,buff,sizeof(CMD::UniCMD));




        
        //在本函数中处理Worker发送数据
        //创建 fifo
        char *path=(char *)malloc(20*sizeof(char));
        sprintf(path,"./fifo/%d",uni_cmd.workerNumber);
        // strcpy(path,"/home/sinow/123");
        errno=0;
        mkfifo(path,S_IRUSR|S_IWUSR);
        // if(mkfifo(path,S_IRUSR|S_IWUSR)==-1&&errno!=EEXIST)
        //     printf("ERROR mkfifo(): %s\n",strerror(errno));
        // else
        //     unlink(path); // 创建该 fifo 成功的 一方 进行 unlink
        
        char *fifo_buff=(char *)malloc(BUF_SZ*sizeof(char));       
        strcpy(fifo_buff,"Hello This is from Your Admin...");
        int fifo_fd=open(path,O_WRONLY);
        if(fifo_fd==-1)
                printf("ERROR open(): %s\n",strerror(errno));
        else
        {
            // 处理，访问数据库
            switch(uni_cmd.action){
                case CMD::CREATE_TB:{
                    if(isLocked&&occupy_worker!=uni_cmd.workerNumber)
                    {
                        strcpy(fifo_buff,"DB on Session");
                        if(write(fifo_fd,fifo_buff,BUF_SZ)==-1)
                            printf("ERROR write(): %s\n",strerror(errno));
                    }
                    else if(!isLocked)
                    {
                        strcpy(fifo_buff,"Open Session First");
                        if(write(fifo_fd,fifo_buff,BUF_SZ)==-1)
                            printf("ERROR write(): %s\n",strerror(errno));
                    }
                    else if((isLocked&&occupy_worker==uni_cmd.workerNumber))
                    {
                        for(int i=0;i<uni_cmd.attr_num;i++)
                        {
                            if(i!=uni_cmd.attr_num-1)
                                uni_cmd.attr[i].next=&uni_cmd.attr[i+1];
                            else
                                uni_cmd.attr[i].next=NULL;
                        }


                        if(dbmgr.CreateTable(uni_cmd.tbname,uni_cmd.attr)==0)
                        {
                            strcpy(fifo_buff,"Create DB Success");
                            if(write(fifo_fd,fifo_buff,BUF_SZ)==-1)
                                printf("ERROR write(): %s\n",strerror(errno));
                        }
                        else{
                            strcpy(fifo_buff,"Create DB Error");
                            if(write(fifo_fd,fifo_buff,BUF_SZ)==-1)
                                printf("ERROR write(): %s\n",strerror(errno));
                        }
                    }
                    



                    
                    break;
                }
                case CMD::SELECT_COL:{

                    SelectCol *col=uni_cmd.select_col;
                    SelectCondi *condi=uni_cmd.condi;
                    for(int i=0;i<uni_cmd.select_col_num;i++)
                    {
                        if(!strcmp(uni_cmd.select_col[0].name,"*"))
                        {
                            col=NULL;
                            break;
                        }
                        if(i!=uni_cmd.select_col_num-1)
                            uni_cmd.select_col[i].next=&uni_cmd.select_col[i+1];
                        else
                            uni_cmd.select_col[i].next=NULL;
                    }

                    if(uni_cmd.condi_num==0)
                    {
                        condi=NULL;
                    }
                    else{
                        for(int i=0;i<uni_cmd.condi_num;i++)
                        {
                            if(i!=uni_cmd.condi_num-1)
                                uni_cmd.condi[i].next=&uni_cmd.condi[i+1];
                            else
                                uni_cmd.condi[i].next=NULL;
                        }
                    }

                    


                    // 如果错误，应该返回错误信息，并由fifo_buff 发送给worker
                    fifo_buff=dbmgr.Select(uni_cmd.tbname,col,condi);
                    if(write(fifo_fd,fifo_buff,BUF_SZ)==-1)
                        printf("ERROR write(): %s\n",strerror(errno));
                    break;
                }
                case CMD::SHOW_TB:{
                    fifo_buff=dbmgr.ShowTables();
                    if(write(fifo_fd,fifo_buff,BUF_SZ)==-1)
                        printf("ERROR write(): %s\n",strerror(errno));
                    break;
                }
                case CMD::SHOW_COL:{
                    fifo_buff=dbmgr.ShowColumns(uni_cmd.tbname);
                    if(write(fifo_fd,fifo_buff,BUF_SZ)==-1)
                        printf("ERROR write(): %s\n",strerror(errno));
                    break;
                }
                case CMD::SHOW_INDEX:{
                    fifo_buff=dbmgr.ShowIndex(uni_cmd.tbname);
                    if(write(fifo_fd,fifo_buff,BUF_SZ)==-1)
                        printf("ERROR write(): %s\n",strerror(errno));
                    break;
                }
                case CMD::INSERT:{

                    if(isLocked&&occupy_worker!=uni_cmd.workerNumber)
                    {
                        strcpy(fifo_buff,"DB on Session");
                        if(write(fifo_fd,fifo_buff,BUF_SZ)==-1)
                            printf("ERROR write(): %s\n",strerror(errno));
                    }
                    else if(!isLocked)
                    {
                        strcpy(fifo_buff,"Open Session First");
                        if(write(fifo_fd,fifo_buff,BUF_SZ)==-1)
                            printf("ERROR write(): %s\n",strerror(errno));
                    }
                    else if((isLocked&&occupy_worker==uni_cmd.workerNumber))
                    {
                        for(int i=0;i<uni_cmd.colval_num;i++)
                        {
                            if(i!=uni_cmd.colval_num-1)
                                uni_cmd.colval[i].next=&uni_cmd.colval[i+1];
                            else
                                uni_cmd.colval[i].next=NULL;
                        }


                        if(dbmgr.Insert(uni_cmd.tbname,uni_cmd.colval)==0)
                        {
                            strcpy(fifo_buff,"Insert Success");
                            if(write(fifo_fd,fifo_buff,BUF_SZ)==-1)
                                printf("ERROR write(): %s\n",strerror(errno));
                        }
                        else{
                            strcpy(fifo_buff,"Insert Error");
                            if(write(fifo_fd,fifo_buff,BUF_SZ)==-1)
                                printf("ERROR write(): %s\n",strerror(errno));
                        }
                    }



                    // if((isLocked&&occupy_worker==uni_cmd.workerNumber)||
                    // !isLocked)
                    // {
                    //     if(dbmgr.Insert(uni_cmd.tbname,uni_cmd.colval)==0)
                    //     {
                    //         strcpy(fifo_buff,"Insert Success");
                    //         if(write(fifo_fd,fifo_buff,BUF_SZ)==-1)
                    //             printf("ERROR write(): %s\n",strerror(errno));
                    //     }
                    //     else{
                    //         strcpy(fifo_buff,"Insert Error");
                    //         if(write(fifo_fd,fifo_buff,BUF_SZ)==-1)
                    //             printf("ERROR write(): %s\n",strerror(errno));
                    //     }
                    // }
                    // else{
                    //     strcpy(fifo_buff,"DB on Session");
                    //     if(write(fifo_fd,fifo_buff,BUF_SZ)==-1)
                    //         printf("ERROR write(): %s\n",strerror(errno));
                    // }
                    
                    break;
                }
                case CMD::UPDATE:{


                    if(isLocked&&occupy_worker!=uni_cmd.workerNumber)
                    {
                        strcpy(fifo_buff,"DB on Session");
                        if(write(fifo_fd,fifo_buff,BUF_SZ)==-1)
                            printf("ERROR write(): %s\n",strerror(errno));
                    }
                    else if(!isLocked)
                    {
                        strcpy(fifo_buff,"Open Session First");
                        if(write(fifo_fd,fifo_buff,BUF_SZ)==-1)
                            printf("ERROR write(): %s\n",strerror(errno));
                    }
                    else if((isLocked&&occupy_worker==uni_cmd.workerNumber))
                    {
                        for(int i=0;i<uni_cmd.condi_num;i++)
                        {
                            if(i!=uni_cmd.condi_num-1)
                                uni_cmd.condi[i].next=&uni_cmd.condi[i+1];
                            else
                                uni_cmd.condi[i].next=NULL;
                        }

                        uint8_t changeNum=0;
                        changeNum=dbmgr.Update(uni_cmd.tbname,uni_cmd.colname,uni_cmd.colval[0].value,uni_cmd.condi);
                        if(changeNum!=0)
                        {
                            sprintf(fifo_buff,"%d Updates\n",changeNum);
                            // strcpy(fifo_buff,"Update Success");
                            if(write(fifo_fd,fifo_buff,BUF_SZ)==-1)
                                printf("ERROR write(): %s\n",strerror(errno));
                        }
                        else{
                            strcpy(fifo_buff,"Update Error");
                            if(write(fifo_fd,fifo_buff,BUF_SZ)==-1)
                                printf("ERROR write(): %s\n",strerror(errno));
                        }
                    }

                    break;
                }
                case CMD::DROPROW:{


                    if(isLocked&&occupy_worker!=uni_cmd.workerNumber)
                    {
                        strcpy(fifo_buff,"DB on Session");
                        if(write(fifo_fd,fifo_buff,BUF_SZ)==-1)
                            printf("ERROR write(): %s\n",strerror(errno));
                    }
                    else if(!isLocked)
                    {
                        strcpy(fifo_buff,"Open Session First");
                        if(write(fifo_fd,fifo_buff,BUF_SZ)==-1)
                            printf("ERROR write(): %s\n",strerror(errno));
                    }
                    else if((isLocked&&occupy_worker==uni_cmd.workerNumber))
                    {

                        for(int i=0;i<uni_cmd.condi_num;i++)
                        {
                            if(i!=uni_cmd.condi_num-1)
                                uni_cmd.condi[i].next=&uni_cmd.condi[i+1];
                            else
                                uni_cmd.condi[i].next=NULL;
                        }
                        uint8_t changeNum=0;
                        changeNum=dbmgr.DropRow(uni_cmd.tbname,uni_cmd.condi);
                        if(changeNum!=0)
                        {
                            strcpy(fifo_buff,"Drop Row Success");
                            if(write(fifo_fd,fifo_buff,BUF_SZ)==-1)
                                printf("ERROR write(): %s\n",strerror(errno));
                        }
                        else{
                            strcpy(fifo_buff,"Drop Row Error");
                            if(write(fifo_fd,fifo_buff,BUF_SZ)==-1)
                                printf("ERROR write(): %s\n",strerror(errno));
                        }
                    }
                    
                    break;
                }
                case CMD::SESSION_OPEN:{
                    if(isLocked)
                    {
                        strcpy(fifo_buff,"DB Session Denied");
                        if(write(fifo_fd,fifo_buff,BUF_SZ)==-1)
                            printf("ERROR write(): %s\n",strerror(errno));
                    }
                    else{
                        occupy_worker=uni_cmd.workerNumber;
                        isLocked=true;
                        strcpy(fifo_buff,"DB Session Allowed");
                        if(write(fifo_fd,fifo_buff,BUF_SZ)==-1)
                            printf("ERROR write(): %s\n",strerror(errno));
                    }
                    break;
                }
                case CMD::SESSION_CLOSE:{
                    if(isLocked)
                    {
                        if(occupy_worker==uni_cmd.workerNumber)
                        {
                            isLocked=false;
                            strcpy(fifo_buff,"DB Session Released");
                            if(write(fifo_fd,fifo_buff,BUF_SZ)==-1)
                                printf("ERROR write(): %s\n",strerror(errno));
                        }
                        else{
                            strcpy(fifo_buff,"No Grants to Release");
                            if(write(fifo_fd,fifo_buff,BUF_SZ)==-1)
                                printf("ERROR write(): %s\n",strerror(errno));
                        }
                    }
                    else{
                        strcpy(fifo_buff,"Already Released");
                        if(write(fifo_fd,fifo_buff,BUF_SZ)==-1)
                            printf("ERROR write(): %s\n",strerror(errno));
                    }
                    break;
                }
                default:{
                    printf("Error Action");
                }
            }

            unlink(path);
        }
        close(fifo_fd);
    }

    free(buff);
    pthread_exit(NULL);
}


void RegisterNofify(mqd_t *msg_q)
{
    printf("Now Registering Notify\n");
    notification=(struct sigevent*)malloc(sizeof(struct sigevent));
    notification->sigev_notify=SIGEV_THREAD;
    notification->sigev_notify_function=MQThread;
    notification->sigev_value.sival_ptr=msg_q;
    
    if(mq_notify(*msg_q,notification)==-1)
    {
        printf("ERROR: %s\n",strerror(errno));
    }
    printf("Registering Notify Finish\n");
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
    printf("Admin Start...\n");
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