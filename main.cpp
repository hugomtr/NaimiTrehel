/*
Authors: 
@Hugo maitre
@Adrien Linares
@Wangyuyang
*/

#include <stdio.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include "main.h"

int writing_in_files(FILE * fp){
    for (int i =0;i<1200;i++){
        fprintf(fp,"Hey bro Thread :%d \n",i);
    }
    return 0;
}

int main(int argc, char *argv[])
{
    int start_port = 0;

    FILE *fp = fopen("trace_execution_algo.txt", "a+");

    if (!fp)
    {
        printf("Error In Opening file");
        exit(EXIT_FAILURE);
    }

    if (argc < 2)
    {
        start_port = 10000;
    }
    else
    {
        start_port = atoi(argv[2]);
    }

    writing_in_files(fp);
    fclose(fp);

    return 0;
}


