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

// VA Globales
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

    int IdInitialJeton = 0;
    run(IdInitialJeton,fp,start_port);

    fclose(fp);

    return 0;
}

// socket serveur qui permettra la récéption des messages des autres sites
int creeSocketReceveur(int myID, int myPort){
    printf("Le Site n°%d créé une socket receveuse n°%hu...", myID, myPort);

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(myPort);

    int sockfd;
    if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0){
        printf("Erreur : Site n°%d : Erreur lors de la création de la socket. Cause : ",myID);
        perror("Socket()");
        exit(EXIT_FAILURE);
    }

    if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0){
        printf("Erreur : Site n°%d : Erreur lors de l'attachement de la socket au port %hu. Cause : ", myID, myPort);
        perror("Bind()");
        exit(EXIT_FAILURE);
    }

    if (listen(sockfd, NB_NODES) < 0){
        printf("Erreur : Erreur lors de la mise en écoute. Cause : ");
        perror("Listen()");
        exit(EXIT_FAILURE);
    }

    return sockfd;
}

void * traitement_message(void * params){
    threadArgs *arg = (struct threadArgs *)params;
    /* For TEST purposes 
    
    // printf("myId %d\n", arg->myID);
    // printf("myPort %d\n", arg->myPort);
    // printf("last %d\n", *(arg->last));
    // printf("next %d\n", *(arg->next));
    
    */

    struct sockaddr_in client;
    socklen_t len = sizeof(struct sockaddr_in);

    message message_buffer; 

    int myID = arg->myID;
    int myPort = arg->myPort;
    int last = *(arg->last);
    int requete_SC = *(arg->requete_SC);
    int token_present = *(arg->token_present);
    int next = *(arg->next);
    FILE * fp = arg->file;
    pthread_mutex_t * adrMutex = arg->mutex;

    int sockreceveur = creeSocketReceveur(myID,myPort);

    int nbSitesTermine = *(arg->nbProcessTermines);
    while(nbSitesTermine<NB_NODES){
        int sockConn = accept(sockreceveur, (struct sockaddr *)&client, &len);
        
        int res = recv(sockConn, &message_buffer, sizeof(message_buffer),0);

        printf("message buffer id request %d\n",message_buffer.idReceveur);
        printf("message buffer id envoyeur %d\n",message_buffer.idEnvoyeur);
        printf("enum message %d\n",message_buffer.type);
        
        if (res > 0){
            switch(message_buffer.type){
                case TOKEN:
                    fprintf(fp,"Site n° %d reçoit le TOKEN du site n° %d\n",myID,message_buffer.idEnvoyeur);
                    pthread_mutex_lock(adrMutex);
                    token_present = TRUE;
                    pthread_mutex_unlock(adrMutex);
                    break;
                case REQUEST:
                    fprintf(fp,"Site n° %d reçoit une REQUEST du site n° %d\n",myID,message_buffer.idEnvoyeur);
                    pthread_mutex_lock(adrMutex);
                    if (last == NIL_PROCESS){
                        if (requete_SC == TRUE) {
                            pthread_mutex_lock(adrMutex);
                            next = message_buffer.idEnvoyeur;
                            pthread_mutex_unlock(adrMutex);                                 
                        }
                        else {
                            fprintf(fp,"Site n° %d envoit le TOKEN à son next le site n° %d\n",myID,next);
                            envoiMessage(myID,TOKEN,next);
                        }
                    }
                    else {
                        fprintf(fp,"Site n° %d envoit une REQUEST à son last le site n° %d\n",myID,last);
                        envoiMessage(myID,REQUEST,last);
                    }
                    last = message_buffer.idEnvoyeur;
                    pthread_mutex_unlock(adrMutex);
                    break;
                case QUIT:
                    fprintf(fp,"Site n° %d à recut QUIT du site n° %d",myID,message_buffer.idEnvoyeur);
                    pthread_mutex_lock(adrMutex);
                    nbSitesTermine += 1;
                    pthread_mutex_unlock(adrMutex);
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

    int nb_entree_SC = arg->nb_entree_SC;
    int myID = arg->myID; 
    int * requete_SC = arg->requete_SC; 
    int * next = arg->next; 
    int * last = arg->last; FILE * fp = arg->file;
    int * token_present = arg->token_present; 
    pthread_mutex_t * mutex = arg->mutex;
    
    for (int i = 0;i< nb_entree_SC;i++){
        desireRentrerenSC(myID, requete_SC,next, last, token_present, mutex);
        fprintf(fp, "Site n°%d entre en SC\n",myID);
        
        sleep(1); //SC
        fprintf(fp, "Site n°%d sort de la SC\n",myID);        
        
        desireSortirSC(myID, requete_SC,next, last, token_present, mutex);
    }

    for (int i = 0; i <NB_NODES;i++) 
    {
        if (i != myID) {
            fprintf(fp,"p%d sends terminate message to %d\n",myID,i);
            envoiMessage(myID,QUIT,i);
        }
    }
    return NULL;
}


void run(int racine_last_id,FILE * fp,int start_port){
    fprintf(fp,"Strart runnning Simulations of Naimi Trehel's Algorithm with %d nodes",NB_NODES);

    int last[NB_NODES];
    int next[NB_NODES]; 
    int requete_SC[NB_NODES]; 
    int token_present[NB_NODES]; 
    int nbProcessTermines[NB_NODES];
    
    for (int i = 0;i<NB_NODES;i++){
        last[i] = racine_last_id;
        next[i] = NIL_PROCESS;
        requete_SC[i] = FALSE;
        token_present[i] = FALSE;
    }
    last[racine_last_id] = NIL_PROCESS;
    token_present[racine_last_id] = TRUE;

    pthread_t thread_receveur[NB_NODES];
    pthread_t thread_travailleur[NB_NODES];
    pthread_mutex_t thread_mutex[NB_NODES];

    threadArgs argReceveur[NB_NODES];

    for (int i = 0;i<NB_NODES;i++){
        argReceveur[i] = {.myID = i, .myPort = start_port + i ,.last = last+i, .next = next+i , .requete_SC = requete_SC+i, 
        .token_present = token_present+i, .nbProcessTermines = nbProcessTermines+i, .file = fp, .mutex = thread_mutex+i};
    }

    for (int i = 0;i<NB_NODES;i++){
        pthread_create(&thread_receveur[i],NULL,traitement_message,&argReceveur[i]);
    }

    threadArgs argTravailleur[NB_NODES];

    for (int i = 0;i<NB_NODES;i++){
        argTravailleur[i] = {.myID = i, .myPort = start_port + i , .nb_entree_SC = 0, .last = last+i, .next = next+i , .requete_SC = requete_SC+i, 
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


int desireRentrerenSC(int myID, int * requete_SC, int * next, int * last, int * has_token, pthread_mutex_t *verrou)
{
	pthread_mutex_lock(verrou);
	*requete_SC = 1;
	if(*last!=-1) // si j'ai un père
	{
		if(envoiMessage(myID,REQUEST,*last)==false) // je lui envoie ma REQUEST
		{
			printf("ERREUR : Site %d n'a pas pu envoyer une demande de SC au site %d\n",myID,*last);
		}
		*last = -1; // je deviens racine de l'anti arborescence
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

int desireSortirSC(int myID, int * requete_SC, int * next, int * last, int * has_token, pthread_mutex_t *verrou)
{
	pthread_mutex_lock(verrou);
    *requete_SC = 0; 
    if (*next !=-1){ // j'ai un next
        if (envoiMessage(myID, TOKEN, *next) == false) // j'envoie le jeton à mon next
        {
            printf(" Erreur : Site %d n'a pas pu envoyer une demande de SC au site %d\n",myID,*next);
        }
    }

	pthread_mutex_unlock(verrou);
    return 0;
}

int envoiMessage(int myID, enum messageType type, int destiID){

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

    message message;
    message.idEnvoyeur = myID;
    message.idReceveur = destiID;
    

    struct sockaddr_in adrReceveur;
    adrReceveur.sin_addr.s_addr = inet_addr("127.0.0.1");
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
    return 0;
}