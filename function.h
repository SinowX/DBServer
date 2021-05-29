#include<string.h>
#include<stdlib.h>
#include<stdio.h>
#include<signal.h>
#include"definition.h"


void pError(int errcode)
{
    printf("ERROR: %s\n",strerror(errcode));
}

