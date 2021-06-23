#include"../database/header/rt_class.h"
#include"../unidef/definition.h"
#include<stdio.h>
#include<string.h>
#include<stdlib.h>

int main()
{

    char *tbname=(char *)malloc(COLUMN_NAME_SIZE*sizeof(char));
    strcpy(tbname,"exp_TB");

    DBMGR Mgr;
    CreateAttr col_1,col_2,col_3;
    strcpy(col_1.name,"column_1");
    strcpy(col_2.name,"column_2");
    strcpy(col_3.name,"column_3");
    
    col_1.flags=ATTR::INT|ATTR::PRIMARY;
    col_2.flags=ATTR::DOUBLE;
    col_3.flags=ATTR::STRING;

    col_1.next=&col_2;
    col_2.next=&col_3;
    col_3.next=NULL;

    // 创建一个包含三列的表
    Mgr.CreateTable(tbname,&col_1);

    InsertColVal insert_1,insert_2,insert_3;
    strcpy(insert_1.name,"column_1");
    strcpy(insert_2.name,"column_2");
    strcpy(insert_3.name,"column_3");

    insert_1.value.i_val=100;
    insert_2.value.d_val=123.00;
    strcpy(insert_3.value.c_val,"This is Another Example String");

    insert_1.flags=ATTR::INT|ATTR::PRIMARY;
    insert_2.flags=ATTR::DOUBLE;
    insert_3.flags=ATTR::STRING;

    insert_1.next=&insert_2;
    insert_2.next=&insert_3;
    insert_3.next=NULL;

    
    //插入示例行数据
    // Mgr.Insert(tbname,&insert_1);


    SelectCol select_1,select_2,select_3;
    strcpy(select_1.name,"column_1");
    strcpy(select_2.name,"column_2");
    strcpy(select_3.name,"column_3");

    select_1.next=&select_2;
    // select_2.next=NULL;

    select_2.next=&select_3;
    select_3.next=NULL;


    // char *result=Mgr.Select(tbname,&select_1,NULL);
    Mgr.Select(tbname,&select_1,NULL);


    char *column_name=(char *)malloc(COLUMN_NAME_SIZE*sizeof(char));

    strcpy(column_name,"column_1");

    val_union colvalue;
    colvalue.i_val=124;

    UpdataCondi condi;

    strcpy(condi.first,"column_2");
    condi.opt=OPT::EQUAL;
    condi.second.d_val=123.0;
    // condi.flags 

    // Mgr.Update(tbname,column_name,colvalue,&condi);

    // Mgr.DropRow(tbname,&condi);

    Mgr.Select(tbname,&select_1,NULL);
    // printf("%s\n",result);


    return 0;
}