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
#include <string.h>
#include <errno.h>
#include <netinet/in.h>

#define NB_SITES_MAX 10


// Lorsqu'on envoi un message, celui-ci est "étiqueté" par son type 
enum typeDuMsg {
    TOKEN, // Message dont l'émetteur est celui qui a le jeton
    REQUEST // Message d'un site souhaitant rentré en SC
};

struct message {
    enum typeDuMsg type; // type du message, défini ci-dessus
    int idEnvoyeur; // ID du site qui envoie le message
    int idReceveur; // ID du site qui fait une requête
    int idDemandeurSC;
};

// Port de départ
int port;


bool envoyerMsg(int myID, int destinationID, typeDuMsg type, int idDemandeurSC){

    printf("Le Site n°%d créé une socket d'envoi vers le Site n°%d...\n", myID, destinationID);
    printf("ok\n");


    message message;
    message.idEnvoyeur = myID;
    message.idReceveur = destinationID;
    message.type = type;
    message.idDemandeurSC = idDemandeurSC;

    printf("message envoyé: {envoyeur: %d, receveur: %d, type: %d, demandeurSc: %d\n",message.idEnvoyeur,message.idReceveur,message.type,message.idDemandeurSC);

    int sockEnvoi;

    struct sockaddr_in server;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_family = AF_INET;
    server.sin_port = htons(3002); // for test purposes

    sockEnvoi = socket(AF_INET, SOCK_STREAM, 0);

    if (sockEnvoi < 0) {
        printf("Erreur lors de la création de la socket\n");
        perror("Socket()");
    }

    if (connect(sockEnvoi, (struct sockaddr*)&server, sizeof(server)) < 0){
        printf("Erreur lors de la connexion au serveur\n");
        perror("Connect()");
    }

    if (send(sockEnvoi, (char*)(&message), sizeof(message), MSG_CONFIRM) < 0){
        printf("Erreur lors de l'envoi du message\n");
        perror("Send()");
    }

    close(sockEnvoi);
    return true;

}


// Test des fct° receive et envoi pour bon fonctionnement
int main(int argc, char const *argv[])
{
    int myID = 2;
    int siteNb = 5;
    port = 3000;

    envoyerMsg(myID, 15, REQUEST,42);
    return 0;
}   



