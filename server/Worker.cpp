#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<errno.h>

#include<sys/socket.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include"function.h"
#include"../unidef/definition.h"
#include<mqueue.h>
#include<fcntl.h>


int curFd=-1;
mqd_t msg_q;

// 解包buff，并将处理后的信息保存到 uni_cmd，成功返回 true，失败返回 false
bool Unpack(char *buff,CMD::UniCMD *uni_cmd)
{
    cmd_pack the_pack;
    printf("%s\n",buff);
    memcpy(&the_pack,buff,sizeof(cmd_pack));
    printf("Action: %s\n",the_pack.act);
    printf("PreObj: %s\n",the_pack.preobj);
    printf("Object: %s\n",the_pack.obj);
    printf("Exec: %s\n",the_pack.exec);
    printf("Condi: %s\n",the_pack.condi);
    printf("Complete.\n");


    printf("%s\n",the_pack.act);
    if(!strcmp(the_pack.act,"create"))
    {
        uni_cmd->action=CMD::CREATE_TB;
        strcpy(uni_cmd->tbname,the_pack.obj);

        
        // 获得了 ; 的位置
        uint8_t semiColonCnt=0;
        int semiColonIndex[MAX_COLUMN];
        
        for(int i=0;i<strlen(the_pack.exec);i++)
        {
            if(the_pack.exec[i]==';')
            {
                semiColonIndex[semiColonCnt]=i;
                semiColonCnt++;
            }
        }

        uni_cmd->attr_num=semiColonCnt;
        char *name_attr=(char *)malloc(BUF_SZ*sizeof(char));
        for(int i=0;i<semiColonCnt;i++)
        {
            if(i==0)
            {
                strncpy(name_attr,&the_pack.exec[0],semiColonIndex[0]-0);
                name_attr[semiColonIndex[0]-0]='\0';
            }
            else
            {
                strncpy(name_attr,&the_pack.exec[semiColonIndex[i-1]+1],semiColonIndex[i]-(semiColonIndex[i-1]+1));
                name_attr[semiColonIndex[i]-(semiColonIndex[i-1]+1)]='\0';
            }

            

            int attr_start_index=0;
            char *attr_str=(char*)malloc(20*sizeof(char));
            for(int j=0;j<strlen(name_attr);j++)
            {
                if(name_attr[j]=='=')
                {
                    attr_start_index=j;
                    strncpy(uni_cmd->attr[i].name,name_attr,j);
                    uni_cmd->attr[i].name[j]='\0';
                }
                else if(name_attr[j]==':')
                {
                    strncpy(attr_str,&name_attr[attr_start_index+1],j-(attr_start_index+1));
                    attr_str[j-(attr_start_index+1)]='\0';
                    attr_start_index=j;
                    if(!strcmp(attr_str,ATTR_INT))
                    {
                        uni_cmd->attr[i].flags|=ATTR::INT;
                    }
                    else if(!strcmp(attr_str,ATTR_DOUBLE))
                    {
                        uni_cmd->attr[i].flags|=ATTR::DOUBLE;
                    }
                    else if(!strcmp(attr_str,ATTR_STRING))
                    {
                        uni_cmd->attr[i].flags|=ATTR::STRING;
                    }
                    else if(!strcmp(attr_str,ATTR_TIME))
                    {
                        uni_cmd->attr[i].flags|=ATTR::TIME;
                    }
                    else if(!strcmp(attr_str,"autoinc"))
                    {
                        uni_cmd->attr[i].flags|=ATTR::AUTOINC;
                    }
                    else if(!strcmp(attr_str,ATTR_PRIMARY))
                    {
                        uni_cmd->attr[i].flags|=ATTR::PRIMARY;
                    }
                }
            }

        }

    
        //处理 the_pack.exe 内容
    }
    else if(!strcmp(the_pack.act,"show"))
    {
        if(!strcmp(the_pack.preobj,"tables"))
        {
            uni_cmd->action=CMD::SHOW_TB;    
        }
        else if(!strcmp(the_pack.preobj,"columns"))
        {
            uni_cmd->action=CMD::SHOW_COL;
            strcpy(uni_cmd->tbname,the_pack.condi);
        }
        else if(!strcmp(the_pack.preobj,"index"))
        {
            uni_cmd->action=CMD::SHOW_INDEX;
            strcpy(uni_cmd->tbname,the_pack.condi);
        }
    }
    else if(!strcmp(the_pack.act,"insert"))
    {
        uni_cmd->action=CMD::INSERT;
        strcpy(uni_cmd->tbname,the_pack.obj);
        //处理 the_pack.exe 内容

        uint8_t semiColonCnt=0;
        int semiColonIndex[MAX_COLUMN];
        for(int i=0;i<strlen(the_pack.exec);i++)
        {
            if(the_pack.exec[i]==';')
            {
                printf("semicolon: %d\n",i);
                semiColonIndex[semiColonCnt]=i;
                semiColonCnt++;
            }
        }

        uni_cmd->colval_num=semiColonCnt;

        // strncpy(name_attr,&the_pack.exec[0],semiColonIndex[i]-0);
        char *name_value=(char *)malloc((ROW_VALUE_SIZE*2)*sizeof(char));

        for(int i=0;i<semiColonCnt;i++)
        {
            if(i==0)
            {
                strncpy(name_value,&the_pack.exec[0],semiColonIndex[0]-0);
                name_value[semiColonIndex[0]]='\0';
            }
            else
            {
                strncpy(name_value,&the_pack.exec[semiColonIndex[i-1]+1],semiColonIndex[i]-semiColonIndex[i-1]-1);
                name_value[semiColonIndex[i]-(semiColonIndex[i-1]+1)]='\0';
            }
            printf("Name-Value: %s\n",name_value);
            // 现在 name_value 保存着 name=value
            char *value_str=(char *)malloc(ROW_VALUE_SIZE*sizeof(char));
            for(int j=0;j<strlen(name_value);j++)
            {
                if(name_value[j]=='=')
                {
                    
                    strncpy(uni_cmd->colval[i].name,&name_value[0],j-0);
                    uni_cmd->colval[i].name[j]='\0';
                    
                    printf("colName: %s\n",uni_cmd->colval[i].name);

                    strncpy(value_str,&name_value[j+1],strlen(name_value)-(j+1));
                    value_str[strlen(name_value)-(j+1)]='\0';
                    
                    printf("value_str: %s\n",value_str);
                    uint8_t value_type=3;
                    for(int k=0;k<strlen(value_str);k++)
                    {
                        // printf(value_str[k])
                        if(value_str[k]>='0'&&value_str[k]<='9')
                            if(value_type!=2)
                                value_type=1;
                        else if(value_str[k]=='.')
                            value_type=2;
                        else
                        {
                            value_type=3;
                            break;
                        }
                    }
                    switch (value_type)
                    {
                    case 1:
                        // int
                        uni_cmd->colval[i].flags=ATTR::INT;
                        memset(&uni_cmd->colval[i].value,0,sizeof(val_union));
                        uni_cmd->colval[i].value.i_val=atoi(value_str);
                        /* code */
                        printf("this is int\n");
                        break;
                    case 2:
                        // double
                        uni_cmd->colval[i].flags=ATTR::DOUBLE;
                        uni_cmd->colval[i].value.d_val=atof(value_str);
                        
                        printf("this is double\n");
                        break;
                    case 3:
                        // string
                        uni_cmd->colval[i].flags=ATTR::STRING;
                        strcpy(uni_cmd->colval[i].value.c_val,value_str);

                        printf("this is string\n");
                        break;
                    default:
                        printf("UNKNOWN ERROR\n");
                        break;
                    }
                    
                    
                    // strncpy(uni_cmd->colval[i].value.c_val,&name_value[j+1],strlen(name_value)-(j+1));
                    // uni_cmd->colval[i].value.c_val[strlen(name_value)-(j+1)]='\0';
                }
            }

        }





    }
    else if(!strcmp(the_pack.act,"update"))
    {
        uni_cmd->action=CMD::UPDATE;
        strcpy(uni_cmd->tbname,the_pack.obj);
        //处理exec condi 内容

        //处理 exec
        uint8_t semiColonCnt=0;
        int semiColonIndex[MAX_COLUMN];
        for(int i=0;i<strlen(the_pack.exec);i++)
        {
            if(the_pack.exec[i]==';')
            {
                semiColonIndex[semiColonCnt]=i;
                semiColonCnt++;
            }
        }

        uni_cmd->colval_num=semiColonCnt;

        // strncpy(name_attr,&the_pack.exec[0],semiColonIndex[i]-0);
        char *name_value=(char *)malloc((ROW_VALUE_SIZE*2)*sizeof(char));

        for(int i=0;i<semiColonCnt;i++)
        {
            if(i==0)
            {
                strncpy(name_value,&the_pack.exec[0],semiColonIndex[0]-0);
                name_value[semiColonIndex[0]]='\0';
            }
            else
            {
                strncpy(name_value,&the_pack.exec[semiColonIndex[i-1]+1],semiColonIndex[i]-semiColonIndex[i-1]-1);
                name_value[semiColonIndex[i]-semiColonIndex[i-1]]='\0';
            }
            // 现在 name_value 保存着 name=value
            // for(int j=0;j<strlen(name_value);j++)
            // {
            //     if(name_value[j]=='=')
            //     {
            //         //value 以 字符串形式保存在 colval[i].value.c_val中，之后由 Admin 进行恢复
            //         strncmp(uni_cmd->colval[i].name,&name_value[0],j-0);
            //         uni_cmd->colval[i].name[j]='\0';
            //         strncmp(uni_cmd->colval[i].value.c_val,&name_value[j+1],strlen(name_value)-(j+1));
            //         uni_cmd->colval[i].value.c_val[strlen(name_value)-(j+1)]='\0';
            //     }
            // }

            char *value_str=(char *)malloc(ROW_VALUE_SIZE*sizeof(char));
            for(int j=0;j<strlen(name_value);j++)
            {
                if(name_value[j]=='=')
                {
                    
                    // strncpy(uni_cmd->colval[i].name,&name_value[0],j-0);
                    // uni_cmd->colval[i].name[j]='\0';
                    strncpy(uni_cmd->colname,&name_value[0],j-0);
                    uni_cmd->colname[j]='\0';
                    
                    strncpy(value_str,&name_value[j+1],strlen(name_value)-(j+1));
                    value_str[strlen(name_value)-(j+1)]='\0';
                    
                    uint8_t value_type=3;
                    for(int k=0;k<strlen(value_str);k++)
                    {
                        if(value_str[i]>='0'&&value_str[i]<='9')
                            value_type=1;
                        else if(value_str[i]=='.')
                            value_type=2;
                        else
                        {
                            value_type=3;
                            break;
                        }
                    }
                    switch (value_type)
                    {
                    case 1:
                        // int
                        uni_cmd->colval[i].flags=ATTR::INT;
                        memset(&uni_cmd->colval[i].value,0,sizeof(val_union));
                        uni_cmd->colval[i].value.i_val=atoi(value_str);
                        /* code */
                        break;
                    case 2:
                        // double
                        uni_cmd->colval[i].flags=ATTR::DOUBLE;
                        uni_cmd->colval[i].value.d_val=atof(value_str);
                        break;
                    case 3:
                        // string
                        uni_cmd->colval[i].flags=ATTR::STRING;
                        strcpy(uni_cmd->colval[i].value.c_val,value_str);
                        break;
                    default:
                        printf("UNKNOWN ERROR\n");
                        break;
                    }
                }
            }
        }


        //condi
        semiColonCnt=0;
        semiColonIndex[MAX_COLUMN];
        for(int i=0;i<strlen(the_pack.condi);i++)
        {
            if(the_pack.condi[i]==';')
            {
                semiColonIndex[semiColonCnt]=i;
                semiColonCnt++;
            }
        }

        uni_cmd->condi_num=semiColonCnt;

        
        // char *name_value=(char *)malloc((ROW_VALUE_SIZE*2)*sizeof(char));

        for(int i=0;i<semiColonCnt;i++)
        {
            if(i==0)
            {
                strncpy(name_value,&the_pack.condi[0],semiColonIndex[0]-0);
                name_value[semiColonIndex[0]]='\0';
            }
            else
            {
                strncpy(name_value,&the_pack.condi[semiColonIndex[i-1]+1],semiColonIndex[i]-(semiColonIndex[i-1]+1));
                name_value[semiColonIndex[i]-(semiColonIndex[i-1]+1)]='\0';
            }
            // 现在 name_value 保存着 name=value
            char *value_str=(char *)malloc(ROW_VALUE_SIZE*sizeof(char));
            for(int j=0;j<strlen(name_value);j++)
            {

                // condi 条件仅限于 < = > ，不支持 <= >=
                if( name_value[j]=='<'||
                    name_value[j]=='='||
                    name_value[j]=='>')
                {
                    //value 以 字符串形式保存在 colval[i].value.c_val中，之后由 Admin 进行恢复
                    strncpy(uni_cmd->condi[i].first,&name_value[0],j-0);
                    uni_cmd->condi[i].first[j]='\0';
                    
                    switch (name_value[j])
                    {
                    case '<':
                        uni_cmd->condi[i].opt=OPT::LESS;
                        break;
                    case '=':
                        uni_cmd->condi[i].opt=OPT::EQUAL;
                        break;
                    case '>':
                        uni_cmd->condi[i].opt=OPT::GREATER;
                        break;
                    default:
                    // 不会跑到default
                        printf("UNKNOWN ERROR");
                        break;
                    }

                    strncpy(value_str,&name_value[j+1],strlen(name_value)-(j+1));
                    value_str[strlen(name_value)-(j+1)]='\0';
                    uint8_t value_type=3;
                    for(int k=0;k<strlen(value_str);k++)
                    {
                        if(value_str[i]>='0'&&value_str[i]<='9')
                            value_type=1;
                        else if(value_str[i]=='.')
                            value_type=2;
                        else
                        {
                            value_type=3;
                            break;
                        }
                    }
                    switch (value_type)
                    {
                    case 1:
                        // int
                        // uni_cmd->condi-
                        uni_cmd->condi[i].flags=ATTR::INT;
                        memset(&uni_cmd->condi[i].second,0,sizeof(val_union));
                        uni_cmd->condi[i].second.i_val=atoi(value_str);
                        /* code */
                        break;
                    case 2:
                        // double
                        uni_cmd->condi[i].flags=ATTR::DOUBLE;
                        uni_cmd->condi[i].second.d_val=atof(value_str);
                        break;
                    case 3:
                        // string
                        uni_cmd->condi[i].flags=ATTR::STRING;
                        strcpy(uni_cmd->condi[i].second.c_val,value_str);
                        break;
                    default:
                        printf("UNKNOWN ERROR\n");
                        break;
                    }



                }
            }

        }




    }
    else if(!strcmp(the_pack.act,"drop"))
    {
        uni_cmd->action=CMD::DROPROW;
        strcpy(uni_cmd->tbname,the_pack.exec);
        //处理 condi

        uint8_t semiColonCnt=0;
        int semiColonIndex[MAX_COLUMN];

        for(int i=0;i<strlen(the_pack.condi);i++)
        {
            if(the_pack.condi[i]==';')
            {
                semiColonIndex[semiColonCnt]=i;
                semiColonCnt++;
            }
        }

        uni_cmd->condi_num=semiColonCnt;

        
        char *name_value=(char *)malloc((ROW_VALUE_SIZE*2)*sizeof(char));

        for(int i=0;i<semiColonCnt;i++)
        {
            if(i==0)
            {
                strncpy(name_value,&the_pack.condi[0],semiColonIndex[0]-0);
                name_value[semiColonIndex[0]]='\0';
            }
            else
            {
                strncpy(name_value,&the_pack.condi[semiColonIndex[i-1]+1],semiColonIndex[i]-(semiColonIndex[i-1]+1));
                name_value[semiColonIndex[i]-(semiColonIndex[i-1]+1)]='\0';
            }
            // 现在 name_value 保存着 name=value
            char *value_str=(char *)malloc(ROW_VALUE_SIZE*sizeof(char));
            for(int j=0;j<strlen(name_value);j++)
            {

                // condi 条件仅限于 < = > ，不支持 <= >=
                if( name_value[j]=='<'||
                    name_value[j]=='='||
                    name_value[j]=='>')
                {
                    //value 以 字符串形式保存在 colval[i].value.c_val中，之后由 Admin 进行恢复
                    strncpy(uni_cmd->condi[i].first,&name_value[0],j-0);
                    uni_cmd->condi[i].first[j]='\0';
                    
                    switch (name_value[j])
                    {
                    case '<':
                        uni_cmd->condi[i].opt=OPT::LESS;
                        break;
                    case '=':
                        uni_cmd->condi[i].opt=OPT::EQUAL;
                        break;
                    case '>':
                        uni_cmd->condi[i].opt=OPT::GREATER;
                        break;
                    default:
                    // 不会跑到default
                        printf("UNKNOWN ERROR");
                        break;
                    }

                    strncpy(value_str,&name_value[j+1],strlen(name_value)-(j+1));
                    value_str[strlen(name_value)-(j+1)]='\0';
                    uint8_t value_type=3;
                    for(int k=0;k<strlen(value_str);k++)
                    {
                        if(value_str[i]>='0'&&value_str[i]<='9')
                            value_type=1;
                        else if(value_str[i]=='.')
                            value_type=2;
                        else
                        {
                            value_type=3;
                            break;
                        }
                    }
                    switch (value_type)
                    {
                    case 1:
                        // int
                        // uni_cmd->condi-
                        uni_cmd->condi[i].flags=ATTR::INT;
                        memset(&uni_cmd->condi[i].second,0,sizeof(val_union));
                        uni_cmd->condi[i].second.i_val=atoi(value_str);
                        /* code */
                        break;
                    case 2:
                        // double
                        uni_cmd->condi[i].flags=ATTR::DOUBLE;
                        uni_cmd->condi[i].second.d_val=atof(value_str);
                        break;
                    case 3:
                        // string
                        uni_cmd->condi[i].flags=ATTR::STRING;
                        strcpy(uni_cmd->condi[i].second.c_val,value_str);
                        break;
                    default:
                        printf("UNKNOWN ERROR\n");
                        break;
                    }



                }
            }

        }


    }
    else if(!strcmp(the_pack.act,"select"))
    {
        uni_cmd->action=CMD::SELECT_COL;
        strcpy(uni_cmd->tbname,the_pack.exec);
        // 处理obj condi

        //obj
        if(!strcmp(the_pack.obj,"*"))
        {
            strcpy(uni_cmd->select_col[0].name,"*");
            uni_cmd->select_col_num=1;
        }
            
        else
        {
            uint colonCnt=0;
            int colonIndex[MAX_COLUMN];
            for(int i=0;i<strlen(the_pack.obj);i++)
            {
                if(the_pack.obj[i]==':')
                {
                    colonIndex[colonCnt]=i;
                    colonCnt++;
                }
            }

            uni_cmd->select_col_num=colonCnt;

            for(int i=0;i<colonCnt;i++)
            {
                if(i==0)
                {
                    strncpy(uni_cmd->select_col[i].name,&the_pack.obj[0],colonIndex[0]-0);
                    uni_cmd->select_col[i].name[colonIndex[0]]='\0';
                }
                else
                {
                    strncpy(uni_cmd->select_col[i].name,&the_pack.obj[colonIndex[i-1]+1],colonIndex[i]-(colonIndex[i-1]+1));
                    uni_cmd->select_col[i].name[colonIndex[i]-(colonIndex[i-1]+1)]='\0';
                }
            }
        }
        
        
        //condi

        printf("the_pack.condi: %s\n",the_pack.condi);

        if(strlen(the_pack.condi)==0)
        {
            //不限制条件
            uni_cmd->condi_num=0;
        }else{
            uint8_t semiColonCnt=0;
            int semiColonIndex[MAX_COLUMN];
            for(int i=0;i<strlen(the_pack.condi);i++)
            {
                if(the_pack.condi[i]==';')
                {
                    semiColonIndex[semiColonCnt]=i;
                    semiColonCnt++;
                }
            }

            uni_cmd->condi_num=semiColonCnt;


            printf("uni_cmd.condi_num: %d\n",uni_cmd->condi_num);
            
            char *name_value=(char *)malloc((ROW_VALUE_SIZE*2)*sizeof(char));

            for(int i=0;i<semiColonCnt;i++)
            {
                if(i==0)
                {
                    strncpy(name_value,&the_pack.condi[0],semiColonIndex[0]-0);
                    name_value[semiColonIndex[0]]='\0';
                }
                else
                {
                    strncpy(name_value,&the_pack.condi[semiColonIndex[i-1]+1],semiColonIndex[i]-(semiColonIndex[i-1]+1));
                    name_value[semiColonIndex[i]-(semiColonIndex[i-1]+1)]='\0';
                }
                printf("Name Value: %s\n",name_value);

                // 现在 name_value 保存着 name=value
                char *value_str=(char *)malloc(ROW_VALUE_SIZE*sizeof(char));
                for(int j=0;j<strlen(name_value);j++)
                {

                    // condi 条件仅限于 < = > ，不支持 <= >=
                    if( name_value[j]=='<'||
                        name_value[j]=='='||
                        name_value[j]=='>')
                    {
                        //value 以 字符串形式保存在 colval[i].value.c_val中，之后由 Admin 进行恢复
                        strncpy(uni_cmd->condi[i].first,&name_value[0],j-0);
                        uni_cmd->condi[i].first[j]='\0';
                        
                        switch (name_value[j])
                        {
                        case '<':
                            uni_cmd->condi[i].opt=OPT::LESS;
                            break;
                        case '=':
                            uni_cmd->condi[i].opt=OPT::EQUAL;
                            break;
                        case '>':
                            uni_cmd->condi[i].opt=OPT::GREATER;
                            break;
                        default:
                        // 不会跑到default
                            printf("UNKNOWN ERROR");
                            break;
                        }

                        strncpy(value_str,&name_value[j+1],strlen(name_value)-(j+1));
                        value_str[strlen(name_value)-(j+1)]='\0';
                        uint8_t value_type=3;
                        for(int k=0;k<strlen(value_str);k++)
                        {
                            if(value_str[i]>='0'&&value_str[i]<='9')
                                value_type=1;
                            else if(value_str[i]=='.')
                                value_type=2;
                            else
                            {
                                value_type=3;
                                break;
                            }
                        }
                        switch (value_type)
                        {
                        case 1:
                            // int
                            // uni_cmd->condi-
                            uni_cmd->condi[i].flags=ATTR::INT;
                            memset(&uni_cmd->condi[i].second,0,sizeof(val_union));
                            uni_cmd->condi[i].second.i_val=atoi(value_str);
                            /* code */
                            break;
                        case 2:
                            // double
                            uni_cmd->condi[i].flags=ATTR::DOUBLE;
                            uni_cmd->condi[i].second.d_val=atof(value_str);
                            break;
                        case 3:
                            // string
                            uni_cmd->condi[i].flags=ATTR::STRING;
                            strcpy(uni_cmd->condi[i].second.c_val,value_str);
                            break;
                        default:
                            printf("UNKNOWN ERROR\n");
                            break;
                        }



                    }
                }

            }




        }

        
    }
    else if(!strcmp(the_pack.act,"session_on"))
    {
        uni_cmd->action=CMD::SESSION_OPEN;
    }
    else if(!strcmp(the_pack.act,"session_off"))
    {
        uni_cmd->action=CMD::SESSION_CLOSE;
    }
    else
    {
        printf("ERROR Unpack()\n");
        return false;
    }

    return true;
}




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
    printf("Now Registering\n");
    struct sigaction *act=(struct sigaction*)malloc(sizeof(struct sigaction));
    act->sa_handler=SecureINTR;

    if(sigaction(SIGINT,act,NULL)==-1)
    {
        printf("ERROR REGISTER SIGACTION: %s\n",strerror(errno));
        exit(0);
    }

    printf("Register Finish\n");
}


mqd_t MQRegister()
{
    printf("Now MQ Registering\n");
    struct mq_attr *attr=(struct mq_attr*)malloc(sizeof(struct mq_attr));
    mqd_t msg_q;
    msg_q=mq_open(MSG_QUEUE_ADDR,O_WRONLY);
    free(attr);
    printf("MQ Register Finish\n");
    return msg_q;
}



int main(int argc, char *argv[])
{
    // printf("This is Workder\n");
    int fd;
    // 将 fd 作为最后一个参数
    if((fd=atoi(argv[argc-1]))==0&&strlen(argv[argc-1])!=1)
        exit(-1); // Exec Not as a Child Process
        
    curFd=fd;

    RegisterSig();

    if((msg_q=MQRegister())==-1)
    {
        printf("ERROR: %s\n",strerror(errno));
        exit(-1);
    }


    char *fifo_buff=(char *)malloc(BUF_SZ*sizeof(char));   
    char *buff=(char *)malloc(BUF_SZ*sizeof(char));

    while(true)
    {
        int stat=read(curFd,buff,sizeof(cmd_pack));
        printf("sizeof)cmd_pack): %d\n",sizeof(cmd_pack));
        printf("read: %d\n",stat);
        if(stat==0||stat==-1)
        {
            exit(stat);
        }else{

            CMD::UniCMD uni_cmd;
            uni_cmd.workerNumber=atoi(argv[0]);

            if(!Unpack(buff,&uni_cmd))
            {
                strcpy(buff,"Broken Pack");
                write(fd,buff,strlen(buff));
                continue;
            }

            mq_send(msg_q,(char *)&uni_cmd,sizeof(CMD::UniCMD),1);
            printf("uni_cmd Has send\n");
            char *path=(char *)malloc(20*sizeof(char));
            sprintf(path,"./fifo/%d",uni_cmd.workerNumber);
            // strcpy(path,"/home/sinow/123");

            // mkfifo(path,S_IRUSR|S_IWUSR);
            if(mkfifo(path,S_IRUSR|S_IWUSR)==-1&&errno!=EEXIST)
                printf("ERROR mkfifo(): %s\n",strerror(errno));
            // else
            //     unlink(path); // 创建该 fifo 的 一方 进行 unlink

                
            int fifo_fd=open(path,O_RDONLY);
            if(fifo_fd==-1)
                printf("ERROR open(): %s\n",strerror(errno));
            else
            {
                if(read(fifo_fd,fifo_buff,BUF_SZ)!=-1)
                {

                    printf("strlen fifo_buff: %d\n",strlen(fifo_buff));
                    for(int i=0;i<100;i++)
                    {
                        printf("%c",fifo_buff[4080+i]);
                    }
                    write(fd,fifo_buff,BUF_SZ);
                }
                printf("%s\n",fifo_buff);
            }
            

            // mq_send(msg_q,buff,BUF_SZ,1);
            // printf("%s",buff);
        }
    }
}