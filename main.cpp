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


// Lorsqu'on envoi un message, celui-ci est "étiqueté" par son type 
enum typeDuMsg {
    AJETON, // Message dont l'émetteur est celui qui a le jeton
    REQUETE // Message d'un site souhaitant rentré en SC
};

typedef struct message {
    enum typeDuMsg tMsg; // type du message, défini ci-dessus
    int idEmetteur; // ID du site qui envoie le message
    int siteReq; // ID du site qui fait une requête
} Mymsg;

// Port de départ
int port;

int writing_in_files(FILE * fp){
    for (int i =0;i<1200;i++){
        fprintf(fp,"Hey bro Thread :%d \n",i);
    }
    return 0;
}

/* Fonction pour créer une socket serveur qui permettra la récéption des messages des autres sites
   Return value : 
   *** Descripteur de fichier de la socket
*/
int creerSocketReceveuse(int myID, int myPort){

    printf("Le Site n°%d créé une socket receveuse n°%hu...", myID, myPort);
    
    // on décrit notre socket d'écoute
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(myPort);

    // on créé la socket receveuse 
    int sockfd;
    if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0){
        printf("Site n°%d : Erreur lors de la création de la socket. Cause : ",myID);
        perror("Socket()");
        exit(1);
    }

    // on attache la socket à un port et une adresse
    if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0){
        printf("Site n°%d : Erreur lors de l'attachement de la socket au port %hu. Cause : \n", myID, myPort);
        perror("Bind()");
        exit(1);
    }

    // on met notre socket en écoute de connexions 
    if (listen(sockfd, NB_SITES_MAX) < 0){
        printf("Erreur lors de la mise en écoute. Cause : ");
        perror("Listen()");
        exit(EXIT_FAILURE);
    }

    return sockfd;
}

/* Cette fonction sert à l'envoi d'un messsage vers un site donné 
    Return values : 
    True -> Le message a été envoyé
    False -> Sinon
*/
bool envoyerMsg(int myID, int destinationID, int whoHasSent, typeDuMsg type){

    printf("Le Site n°%d créé une socket d'envoi vers le Site n°%d...\n", myID, destinationID);

    /* Avant de poursuivre dans la fonction, il est important de s'assurer
    que le site émetteur ne soit pas également le site récépteur */
    if (myID == destinationID){
        printf("Le site envoyant le message est le même que celui recevant le message.\
                Arrêt du programme\n.");
        exit(1);
    }

    Mymsg message;
    message.idEmetteur = myID;
    message.tMsg = type;

    int sockEnvoi;

    struct sockaddr_in server;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    sockEnvoi = socket(AF_INET, SOCK_STREAM, 0);

    if (sockEnvoi < 0) {
        printf("Erreur lors de la création de la socket\n");
        perror("Socket()");
        exit(1);
        return false;
    }

    if (connect(sockEnvoi, (struct sockaddr*)& server, sizeof(server)) < 0){
        printf("Erreur lors de la connexion au serveur\n");
        perror("Connect()");
        exit(1);
        return false;
    }

    if (send(sockEnvoi, (char*)(&message), sizeof(message), MSG_CONFIRM) < 0){
        printf("Erreur lors de l'envoi du message\n");
        perror("Send()");
        exit(1);
        return false;
    }

    close(sockEnvoi);
    return true;

}


// int main(int argc, char *argv[])
// {
//     int start_port = 0;

//     FILE *fp = fopen("trace_execution_algo.txt", "a+");

//     if (!fp)
//     {
//         printf("Error In Opening file");
//         exit(EXIT_FAILURE);
//     }

//     if (argc < 2)
//     {
//         start_port = 10000;
//     }
//     else
//     {
//         start_port = atoi(argv[2]);
//     }

//     writing_in_files(fp);
//     fclose(fp);

//     return 0;
// }


// Test des fct° receive et envoi pour bon fonctionnement
int main(int argc, char const *argv[])
{
int siteNb;  

if (argc != 3)
    {
        fprintf(stderr, "Usage: %s numberSite, port\n", argv[0]);
        exit(1);
    }


    // On attend 2 arguments : l'ID du site et le n° de port
    siteNb = atoi(argv[1]); 
    port = (unsigned short) atoi(argv[2]);
    
    // int sfd = creerSocketReceveuse(siteNb, port+siteNb);
    // printf("\nFile descriptor returned: %d\n", sfd);

    bool val = envoyerMsg(siteNb, 5, siteNb, REQUETE);
    printf("value returned by envoyerMsg: %d", val);


}   



