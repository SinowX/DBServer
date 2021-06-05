#ifndef _CLIENTDEF
#define _CLIENTDEF

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 5000
#define BUF_SZ  256

typedef struct cmd_pack{
    char act[BUF_SZ];
    char preobj[BUF_SZ];
    char obj[BUF_SZ];
    char exec[BUF_SZ];
    char condi[BUF_SZ];
}cmd_pack;


#endif