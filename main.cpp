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

void * traitement_message(void * params){
    threadArgs *arg = (struct threadArgs *)params;
    printf("myId %d\n", arg->myID);
    printf("myPort %d\n", arg->myPort);
    printf("last %d\n", *(arg->last));
    printf("next %d\n", *(arg->next));
    return NULL;
}

void * travail(void * args){
    return 0;
}


int run(int racine_last_id,FILE * fp,int start_port){
    fprintf(fp,"Strart runnning Simulations of Naimi Trehel's Algorithm with %d nodes",NB_NODES);

    int last[NB_NODES];
    int next[NB_NODES]; 
    int requete_CS[NB_NODES]; 
    int token_present[NB_NODES]; 
    
    for (int i = 0;i<NB_NODES;i++){
        last[i] = racine_last_id;
        next[i] = NIL_PROCESS;
        requete_CS[i] = 0;
        token_present[i] = 0;
    }
    token_present[racine_last_id] = 1;

    pthread_t thread_receveur[NB_NODES];
    pthread_t thread_travailleur[NB_NODES];

    threadArgs arg[NB_NODES];

    for (int i = 0;i<NB_NODES;i++){
        arg[i] = {.myID = i, .myPort = start_port + i ,.last = last+i, .next = next+i , .requete_CS = requete_CS+i, .token_present = token_present+i};
    }
    
    for (int i = 0;i<NB_NODES;i++){
        pthread_create(&thread_receveur[i],NULL,traitement_message,&arg[i]);
    }

    for (int i = 0;i<NB_NODES;i++){
        pthread_create(&thread_travailleur[i],NULL,travail,NULL);
    }

    for (int i = 0;i<NB_NODES;i++){
        pthread_join(thread_receveur[i],NULL);
        pthread_join(thread_travailleur[i],NULL);
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

    int nodeIdInitialJeton = 0;
    run(nodeIdInitialJeton,fp,start_port);

    fclose(fp);

    return 0;
}


