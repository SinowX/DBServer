#include<stdio.h>
#include<stdlib.h>


int main(int argc, char *argv[])
{
    char * str=(char *)malloc(256*sizeof(char));
    fgets(str,255,stdin);
    printf("%s\n",str);
    return 0;
}