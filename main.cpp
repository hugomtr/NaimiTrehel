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
#include <string.h>
#include <errno.h>
#include <netinet/in.h>

#define NB_SITES_MAX 10

int writing_in_files(FILE * fp){
    for (int i =0;i<1200;i++){
        fprintf(fp,"Hey bro Thread :%d \n",i);
    }
    return 0;
}


// socket serveur qui permettra la récéption des messages des autres sites
int creerSocketReceveuse(int myID, int myPort){
    
    printf("Le Site n°%d créé une socket receveuse n°%hu...", myID, myPort);
    struct sockaddr_in server;

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(myPort);

    // on créé la socket receveuse 
    int sockfd;
    if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0){
        printf("Erreur : Site n°%d : Erreur lors de la création de la socket. Cause : ",myID);
        perror("Socket()");
        exit(1);
    }

    // on attache la socket à un port et une adresse
    if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0){
        printf("Erreur : Site n°%d : Erreur lors de l'attachement de la socket au port %hu. Cause : ", myID, myPort);
        perror("Bind()");
        exit(1);
    }

    // on met notre socket en écoute de connexions 
    if (listen(sockfd, NB_SITES_MAX) < 0){
        printf("Erreur : Erreur lors de la mise en écoute. Cause : ");
        perror("Listen()");
        exit(EXIT_FAILURE);
    }

    return sockfd;
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


// Test de la fct° socketReceive pour bon fonctionnement
// int main(int argc, char const *argv[])
// {
// unsigned short port; 
// int siteNb;  

// if (argc != 3)
//     {
//         fprintf(stderr, "Usage: %s numberSite, port\n", argv[0]);
//         exit(1);
//     }

//     siteNb = atoi(argv[1]);
//     port = (unsigned short) atoi(argv[2]);
    
//     int sfd = creerSocketReceveuse(siteNb, port);
//     printf("\nFile descriptor returned: %d", sfd);


// }


