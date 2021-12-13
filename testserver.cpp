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
int port_depart;

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


int creerSocketReceveuse(int myID, int myPort){

    printf("Le Site n°%d créé une socket receveuse n°%d...", myID, myPort);
    
    // on décrit notre socket d'écoute
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(3002);// for test purposes

    // on créé la socket receveuse 
    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (sockfd < 0){
        printf("Site n°%d : Erreur lors de la création de la socket. Cause : ",myID);
        perror("Socket()");
        exit(1);
    }

    // on attache la socket à un port et une adresse
    if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0){
        printf("Site n°%d : Erreur lors de l'attachement de la socket au port %d. Cause : \n", myID, myPort);
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


int main()
{
    port_depart = 3000;
    int myID = 2;
    int socketServeur = creerSocketReceveuse(myID,port_depart+myID);

    sockaddr_in addrClient;
    socklen_t len = sizeof(struct sockaddr_in);

    message message_buffer;
    
    int sockConn = accept(socketServeur, (struct sockaddr *)&addrClient, &len);

    int res = recv(sockConn, &message_buffer, sizeof(message_buffer),0);

    printf("message recu: {envoyeur: %d, receveur: %d, type: %d, demandeurSc: %d\n",message_buffer.idEnvoyeur,message_buffer.idReceveur,message_buffer.type,message_buffer.idDemandeurSC);

    close(sockConn);
    close(socketServeur);
}