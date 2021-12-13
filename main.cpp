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
    FILE *fp = fopen("trace_execution_algo.txt", "a+");

    if (!fp)
    {
        printf("Error In Opening file");
        exit(EXIT_FAILURE);
    }

    if (argc < 2)
    {
        start_port = 3000;
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

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(myPort);

    int sockfd;
    if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0){
        printf("Erreur : Site n°%d : Erreur lors de la création de la socket. Cause : ",myID);
        perror("Socket()");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0){
        printf("Erreur : Site n°%d : Erreur lors de l'attachement de la socket au port %d. Cause : ", myID, myPort);
        perror("Bind()");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (listen(sockfd, NB_NODES) < 0){
        printf("Erreur : Erreur lors de la mise en écoute. Cause : ");
        perror("Listen()");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Socket receveur crée sur le site %d au port %d\n",myID,myPort);
    return sockfd;
}

void * traitement_message(void * params){
    threadArgs *arg = (struct threadArgs *)params;

    struct sockaddr_in client;
    socklen_t len = sizeof(struct sockaddr_in);

    message message_buffer; 

    int myID = arg->myID;
    int myPort = arg->myPort;
    int * last = arg->last;
    int * requete_SC = arg->requete_SC;
    int * token_present = arg->token_present;
    int * next = arg->next;
    FILE * fp = arg->file;
    pthread_mutex_t * adrMutex = arg->mutex;

    int sockreceveur = creeSocketReceveur(myID,myPort);
    printf("Le site n° %d commence à ecouter les connections\n",myID);

    int * nbSitesTermine = arg->nbProcessTermines;
    while(*nbSitesTermine<NB_NODES){
        int sockConn = accept(sockreceveur, (struct sockaddr *)&client, &len);
        if (sockConn < 0){
            printf("Erreur accept socket receveur du site %d\n",myID);
            perror("Accept()");
            close(sockConn);
        }
        
        int res;        
        do {
            int res = recv(sockConn, &message_buffer, sizeof(message_buffer),0);
            printf("message recu au site %d: {envoyeur: %d, receveur: %d, type: %d, demandeurSc: %d\n",myID,message_buffer.idEnvoyeur,message_buffer.idReceveur,message_buffer.type,message_buffer.idDemandeurSC);

            if ( res > 0 )
                switch(message_buffer.type){
                    case TOKEN:
                        printf("Site n° %d reçoit le TOKEN du site n° %d\n",myID,message_buffer.idEnvoyeur);
                        fprintf(fp,"Site n° %d reçoit le TOKEN du site n° %d\n",myID,message_buffer.idEnvoyeur);
                        pthread_mutex_lock(adrMutex);
                        *token_present = TRUE;
                        pthread_mutex_unlock(adrMutex);
                        break;
                    case REQUEST:
                        printf("Site n° %d reçoit une REQUEST du site n° %d\n",myID,message_buffer.idEnvoyeur);
                        fprintf(fp,"Site n° %d reçoit une REQUEST du site n° %d\n",myID,message_buffer.idEnvoyeur);
                        pthread_mutex_lock(adrMutex);
                        if (*last == NIL_PROCESS){
                            if (*requete_SC == TRUE) {
                                pthread_mutex_lock(adrMutex);
                                *next = message_buffer.idDemandeurSC;
                                pthread_mutex_unlock(adrMutex);                                 
                            }
                            else {
                                printf("Site n° %d envoit le TOKEN à son next le site n° %d\n",myID,message_buffer.idDemandeurSC);
                                fprintf(fp,"Site n° %d envoit le TOKEN à son next le site n° %d\n",myID,message_buffer.idDemandeurSC);
                                envoiMessage(myID,TOKEN,message_buffer.idDemandeurSC,NIL_PROCESS);
                            }
                        }
                        else {
                            printf("Site n° %d fait passer la REQUEST à son last le site n° %d\n",myID,*last);
                            fprintf(fp,"Site n° %d fait passer la REQUEST à son last le site n° %d\n",myID,*last);
                            envoiMessage(myID,REQUEST,*last,message_buffer.idDemandeurSC); 
                        }
                        *last = message_buffer.idEnvoyeur;
                        pthread_mutex_unlock(adrMutex);
                        break;
                    case QUIT:
                        printf("Site n° %d à recut QUIT du site n° %d",myID,message_buffer.idEnvoyeur);
                        fprintf(fp,"Site n° %d à recut QUIT du site n° %d",myID,message_buffer.idEnvoyeur);
                        pthread_mutex_lock(adrMutex);
                        *nbSitesTermine += 1;
                        pthread_mutex_unlock(adrMutex);
                        break;
                    default:
                        printf("Error : message de type non reconnu\n");
                        exit(EXIT_FAILURE);
                        break;
                }
            
            else if ( res == 0 )
                printf("Connection closed\n");
            
            else
                printf("recv failed par le site %d\n", myID);
                perror("Recv()");

        } while( res > 0 );
    }
    pthread_exit(NULL); 
}

void * travail(void * params){
    threadArgs *arg = (struct threadArgs *)params;
    printf("Thread Travail n°%d\n",arg->myID);

    int nb_entree_SC = 0;
    int myID = arg->myID; 
    int * requete_SC = arg->requete_SC; 
    int * next = arg->next; 
    int * last = arg->last; FILE * fp = arg->file;
    int * token_present = arg->token_present; 
    pthread_mutex_t * mutex = arg->mutex;
    
    for (nb_entree_SC = 0;nb_entree_SC< NB_EXEC_SC;nb_entree_SC++){
        desireRentrerenSC(myID, requete_SC,next, last, token_present, mutex);
        
        printf("Site n°%d entre en SC pour la %d ième fois en SC\n",myID,nb_entree_SC+1);
        fprintf(fp, "Site n°%d entre en SC pour la %d ième fois en SC\n",myID,nb_entree_SC+1);
        
        sleep(1); //SC
        printf("Site n°%d sort de la SC\n",myID);
        fprintf(fp, "Site n°%d sort de la SC\n",myID);        
        desireSortirSC(myID, requete_SC,next, last, token_present, mutex);
    }

    for (int i = 0; i <NB_NODES;i++) 
    {
        if (i != myID) {
            fprintf(fp,"p%d sends terminate message to %d\n",myID,i);
            envoiMessage(myID,QUIT,i,NIL_PROCESS);
        }
    }
    pthread_exit(NULL); 
}


void run(int racine_last_id,FILE * fp,int start_port){
    fprintf(fp,"Strart runnning Simulations of Naimi Trehel's Algorithm with %d nodes\n",NB_NODES);

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

    for (int i=0;i<NB_NODES;i++)
    {
        thread_mutex[i] = PTHREAD_MUTEX_INITIALIZER;
    }

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
        argTravailleur[i] = {.myID = i, .myPort = start_port + i, .last = last+i, .next = next+i , .requete_SC = requete_SC+i, 
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
    printf("Site n°%d desireRentreSC son next est%d\n",myID,*next);
	pthread_mutex_lock(verrou);
	*requete_SC = TRUE;
	if(*last!=-1) // si j'ai un père
	{
		if (envoiMessage(myID,REQUEST,*last,myID) == FALSE) {
            printf("Erreur sur l'envoi de request du site %d au site %d",myID,*last);
        }; 
		*last = NIL_PROCESS; // je deviens racine de l'anti arborescence
	}
    
	while(1) 
	{
        pthread_mutex_lock(verrou);
		if(*has_token) 
		{
			pthread_mutex_unlock(verrou);
			break;
		}
        pthread_mutex_unlock(verrou);        
	}
    return 0;
}

int desireSortirSC(int myID, int * requete_SC, int * next, int * last, int * has_token, pthread_mutex_t *verrou)
{
	pthread_mutex_lock(verrou);
    *requete_SC = FALSE; 
    if (*next !=-1){ // j'ai un next
        if (envoiMessage(myID, TOKEN, *next, NIL_PROCESS) == FALSE)
        {
            printf("Erreur sur l'envoi de jeton du site %d au site %d",myID,*next);
        } 
    }
    printf("%d cède le jeton au site %d\n",myID,*next);
	pthread_mutex_unlock(verrou);
    return 0;
}

int envoiMessage(int myID, enum messageType type, int destiID, int idDemandeurSC){

    /* Avant de poursuivre dans la fonction, il est important de s'assurer
    que le site émetteur ne soit pas également le site récépteur */
    if (myID == destiID){
        printf("Le site envoyant le message est le même que celui recevant le message.\
                Arrêt du programme\n.");
        exit(EXIT_FAILURE);
    }

    int sockEnvoi = socket(AF_INET,SOCK_STREAM,0);

    if (sockEnvoi < 0) {
        printf("Erreur lors de la création de la socket\n");
        perror("Socket()");
        return FALSE;
    }

    message message;
    message.idDemandeurSC = idDemandeurSC;
    message.idEnvoyeur = myID;
    message.idReceveur = destiID;
    message.type = type;

    struct sockaddr_in adrReceveur;
    adrReceveur.sin_addr.s_addr = inet_addr("127.0.0.1");
    adrReceveur.sin_family = AF_INET;
    adrReceveur.sin_port = htons(start_port + destiID);

    if (connect(sockEnvoi, (struct sockaddr*)& adrReceveur, sizeof(adrReceveur)) < 0){
        printf("Erreur lors de la connexion a serveur au niveau du site %d sur le port %d\n",myID,start_port + destiID);
        perror("Connect()");
        return FALSE;
    }

    if (send(sockEnvoi, (char* )&message, sizeof(message), MSG_CONFIRM) < 0){
        printf("Erreur lors de l'envoi du message\n");
        perror("Send()");
        return FALSE;
    }

    printf("message envoyé du site:%d {envoyeur: %d, receveur: %d, type: %d, demandeurSc: %d\n",myID,message.idEnvoyeur, message.idReceveur,message.type,message.idDemandeurSC);

    close(sockEnvoi);
    return TRUE;
}
