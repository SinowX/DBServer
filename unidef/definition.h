#ifndef _DEFINATION
#define _DEFINATION

#include"../database/header/db_limits.h"
#include"../database/header/sql_struct.h"
#include"stdint.h"

enum{
    SIG_SESSION_REQUSET_OPEN,
    SIG_SESSION_REQUEST_CLOSE,
    SIG_SESSION_REPLY_ALLOW,
    SIG_SESSION_REPLY_DENY,
    
    
};

typedef struct cmd_pack{
    char act[256];
    char preobj[256];
    char obj[256];
    char exec[256];
    char condi[256];
}cmd_pack;


//worker 向Admin 发送的指令类别
namespace CMD{
    enum{
        CREATE_TB,
        SELECT_COL,
        SHOW_TB,
        SHOW_COL,
        SHOW_INDEX,
        INSERT,
        UPDATE,
        DROPROW,
        SESSION_OPEN,
        SESSION_CLOSE
    };

    typedef struct UniCMD{
        uint workerNumber;
        uint8_t action;
        char tbname[TABLE_NAME_SIZE];
        CreateAttr attr[MAX_COLUMN];
        uint8_t attr_num;
        SelectCol select_col[MAX_COLUMN];
        uint8_t select_col_num;
        char colname[COLUMN_NAME_SIZE];
        
        Condi condi[MAX_COLUMN];
        uint8_t condi_num;
        // colval[0].value 可用于 update 使用
        InsertColVal colval[MAX_COLUMN];
        uint8_t colval_num;
    }UniCMD;

}






#define MSG_QUEUE_ADDR "/W2A_Queue"
#define MSG_QUEUE_MAXMSG 10
#define MSG_QUEUE_MSGSIZE 8192 // linux sys limits

#define LISTEN_PORT 5000
#define LISTEN_ADDR "0.0.0.0"
#define MAX_CLIENT 256
#define BUF_SZ 8192


#endif