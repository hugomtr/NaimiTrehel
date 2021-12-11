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
    enum typeDuMsg tMsg; // type du message, défini ci-dessus
    int idEnvoyeur; // ID du site qui envoie le message
    int idRequest; // ID du site qui fait une requête
};

// Port de départ
int port;


bool envoyerMsg(int myID, int destinationID, typeDuMsg type){

    printf("Le Site n°%d créé une socket d'envoi vers le Site n°%d...\n", myID, destinationID);

    /* Avant de poursuivre dans la fonction, il est important de s'assurer
    que le site émetteur ne soit pas également le site récépteur */
    if (myID == destinationID){
        printf("Le site envoyant le message est le même que celui recevant le message.\
                Arrêt du programme\n.");
        exit(1);
    }

    message message;
    message.idEnvoyeur = 4000002;
    message.tMsg = type;
    message;

    int sockEnvoi;

    struct sockaddr_in server;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_family = AF_INET;
    server.sin_port = htons(3000); // for test purposes

    sockEnvoi = socket(AF_INET, SOCK_STREAM, 0);

    if (sockEnvoi < 0) {
        printf("Erreur lors de la création de la socket\n");
        perror("Socket()");
        exit(1);
        return false;
    }

    if (connect(sockEnvoi, (struct sockaddr*)&server, sizeof(server)) < 0){
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


// Test des fct° receive et envoi pour bon fonctionnement
int main(int argc, char const *argv[])
{
    int siteNb = 5;
    port = 3000;

    bool val = envoyerMsg(2, 5, TOKEN);
}   



