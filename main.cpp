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

int start_port;

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
        exit(EXIT_FAILURE);
    }

    // on attache la socket à un port et une adresse
    if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0){
        printf("Erreur : Site n°%d : Erreur lors de l'attachement de la socket au port %hu. Cause : ", myID, myPort);
        perror("Bind()");
        exit(EXIT_FAILURE);
    }

    // on met notre socket en écoute de connexions 
    if (listen(sockfd, 20) < 0){
        printf("Erreur : Erreur lors de la mise en écoute. Cause : ");
        perror("Listen()");
        exit(EXIT_FAILURE);
    }

    return sockfd;
}

void * traitement_message(void * params){
    threadArgs *arg = (struct threadArgs *)params;
    printf("myId %d\n", arg->myID);
    printf("myPort %d\n", arg->myPort);
    printf("last %d\n", *(arg->last));
    printf("next %d\n", *(arg->next));


    struct sockaddr_in client;
    socklen_t len = sizeof(struct sockaddr_in);

    message message_buffer; 

    int myID = arg->myID;
    int myPort = arg->myPort;
    int sockreceveur = creeSocketReceveur(myID,myPort);

    int nbSitesTermine = *(arg->nbProcessTermines);
    while(nbSitesTermine<NB_NODES){
        int sockConn = accept(sockreceveur, (struct sockaddr *)&client, &len);
        if (recv(sockConn, &message_buffer, sizeof(message_buffer),0)>0){
            switch(message_buffer.type){
                case TOKEN:
                    break;
                default:
                    printf("Error : message de type non reconnu\n");
                    exit(EXIT_FAILURE);
                    break;
            }
        }
    }
    return NULL;
}

void * travail(void * params){
    threadArgs *arg = (struct threadArgs *)params;
    printf("myId %d\n", arg->myID);
    printf("myPort %d\n", arg->myPort);
    printf("last %d\n", *(arg->last));
    printf("next %d\n", *(arg->next));

    int nb_entree_CS = arg->nb_entree_CS;
    int myID = arg->myID;
    for (int i = 0;i< nb_entree_CS;i++){
        int myId = arg->myID; int * requete_CS = arg->requete_CS; int * next = arg->next; int * last = arg->last; 
        int * token_present = arg->token_present; pthread_mutex_t * mutex = arg->mutex;
        desireRentrerenSC(myID, requete_CS,next, last, token_present, mutex);
        sleep(1); //CS
        desireSortirSC(myID, requete_CS,next, last, token_present, mutex);
    }
    return 0;
}


void run(int racine_last_id,FILE * fp,int start_port){
    fprintf(fp,"Strart runnning Simulations of Naimi Trehel's Algorithm with %d nodes",NB_NODES);

    int last[NB_NODES];
    int next[NB_NODES]; 
    int requete_CS[NB_NODES]; 
    int token_present[NB_NODES]; 
    int nbProcessTermines[NB_NODES];
    
    for (int i = 0;i<NB_NODES;i++){
        last[i] = racine_last_id;
        next[i] = NIL_PROCESS;
        requete_CS[i] = 0;
        token_present[i] = 0;
    }
    last[racine_last_id] = NIL_PROCESS;
    token_present[racine_last_id] = 1;

    pthread_t thread_receveur[NB_NODES];
    pthread_t thread_travailleur[NB_NODES];
    pthread_mutex_t thread_mutex[NB_NODES];

    threadArgs argReceveur[NB_NODES];

    for (int i = 0;i<NB_NODES;i++){
        argReceveur[i] = {.myID = i, .myPort = start_port + i ,.last = last+i, .next = next+i , .requete_CS = requete_CS+i, 
        .token_present = token_present+i, .nbProcessTermines = nbProcessTermines+i, .file = fp, .mutex = thread_mutex+i};
    }

    for (int i = 0;i<NB_NODES;i++){
        pthread_create(&thread_receveur[i],NULL,traitement_message,&argReceveur[i]);
    }

    threadArgs argTravailleur[NB_NODES];

    for (int i = 0;i<NB_NODES;i++){
        argTravailleur[i] = {.myID = i, .myPort = start_port + i , .nb_entree_CS = 0, .last = last+i, .next = next+i , .requete_CS = requete_CS+i, 
        .token_present = token_present+i, .nbProcessTermines = nbProcessTermines+i, .file = fp, .mutex = thread_mutex+i};
    }

    for (int i = 0;i<NB_NODES;i++){
        pthread_create(&thread_travailleur[i],NULL,travail,&argTravailleur[i]);
    }

    for (int i = 0;i<NB_NODES;i++){
        pthread_join(thread_receveur[i],NULL);
        pthread_join(thread_travailleur[i],NULL);
    }

    for (int i = 0;i<NB_NODES;i++){
        pthread_mutex_destroy(&thread_mutex[i]);
    }
}


int desireRentrerenSC(int myID, int * requete_CS, int * next, int * last, int * has_token, pthread_mutex_t *verrou)
{
	pthread_mutex_lock(verrou);
	*requete_CS = 1;
	if(*last!=-1) // si j'ai un père
	{
		if(envoiMessage(myID,REQUEST,*last)==false) // je lui envoie ma REQUEST
		{
			printf("ERREUR : Site %d n'a pas pu envoyer une demande de SC au site %d\n",myID,*last);
		}
		*last = -1;
	}
    
	while(1) 
	{
		if(has_token) 
		{
			pthread_mutex_unlock(verrou);
			break;
		}
	}
	pthread_mutex_unlock(verrou);
    return 0;
}

int desireSortirSC(int myID, int * requete_CS, int * next, int * last, int * has_token, pthread_mutex_t *verrou)
{
	pthread_mutex_lock(verrou);
    *requete_CS = 0; 
    if (*next !=-1){ // j'ai un next
        if (envoiMessage(myID, TOKEN, *next) == false) // j'envoie le jeton à mon next
        {
            printf(" Erreur : Site %d n'a pas pu envoyer une demande de SC au site %d\n",myID,*next);
        }
    }

	pthread_mutex_unlock(verrou);
    return 0;
}

int envoimesage(int myID, enum messageType message, int destiID){

    printf("Le Site n°%d créé une socket d'envoi vers le Site n°%d...", myID, destiID);

    /* Avant de poursuivre dans la fonction, il est important de s'assurer
    que le site émetteur ne soit pas également le site récépteur */
    if (myID == destiID){
        printf("Le site envoyant le message est le même que celui recevant le message.\
                Arrêt du programme\n.");
        exit(1);
    }

    int sockEnvoi = socket(AF_INET,SOCK_STREAM,0);

    if ( socket(sockEnvoi, SOCK_STREAM, 0) < 0) {
        printf("Erreur lors de la création de la socket\n");
        perror("Socket()");
        exit(1);
        return false;
    }

    char message;

    struct sockaddr_in adrReceveur;
    adrReceveur.sin_addr.s_addr = INADDR_ANY;
    adrReceveur.sin_family = PF_INET;
    adrReceveur.sin_port = htons(start_port+destiID);

    if (connect(sockEnvoi, (struct sockaddr*)& adrReceveur, sizeof(adrReceveur)) < 0){
        printf("Erreur lors de la connexion au serveur\n");
        perror("Connect()");
        exit(1);
        return false;
    }

    if (send(sockEnvoi, (char* )&message, sizeof(message), MSG_CONFIRM) < 0){
        printf("Erreur lors de l'envoi du message\n");
        perror("Send()");
        exit(1);
        return false;
    }

    close(sockEnvoi);
    return true;
}