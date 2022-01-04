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
#include "calcul.h"

// VA Globales
int start_port;
int NB_NODES;
int NB_EXEC_SC;
int myID;

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Args : id_du_site nb_exec_site");
        exit(EXIT_FAILURE);
    } else if (argc == 5) {
        start_port = atoi(argv[1]);
        myID = atoi(argv[2]);
        NB_EXEC_SC = atoi(argv[3]);
        NB_NODES = atoi(argv[4]);
    } else {
        start_port = 3000;
        myID = atoi(argv[1]);
        NB_EXEC_SC = atoi(argv[2]);
        NB_NODES = 4;
    }

    run();
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

    //printf("Socket receveur crée sur le site %d au port %d\n",myID,myPort);
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
    pthread_mutex_t * adrMutex = arg->mutex;

    int sockreceveur = creeSocketReceveur(myID,myPort);
    //printf("Le site n° %d commence à ecouter les connections\n",myID);

    int * nbSitesTermine = arg->nbProcessTermines;

    while(*nbSitesTermine<NB_NODES-1){
        int sockConn = accept(sockreceveur, (struct sockaddr *)&client, &len);
        if (sockConn < 0){
            printf("Erreur accept socket receveur du site %d\n",myID);
            perror("Accept()");
            close(sockConn);
        }
        
        int res;        
        do {
            int res = recv(sockConn, &message_buffer, sizeof(message_buffer),0);
            //printf("message recu au site %d: {envoyeur: %d, receveur: %d, type: %d, demandeurSc: %d\n",myID,message_buffer.idEnvoyeur,message_buffer.idReceveur,message_buffer.type,message_buffer.idDemandeurSC);

            if ( res > 0 )
                switch(message_buffer.type){
                    case TOKEN:
                        pthread_mutex_lock(adrMutex);
                        printf("Site n° %d reçoit le TOKEN du site n° %d\n",myID,message_buffer.idEnvoyeur);
                        *token_present = TRUE;
                        pthread_mutex_unlock(adrMutex);
                        break;
                    case REQUEST:
                        pthread_mutex_lock(adrMutex);
                        printf("Site n° %d reçoit une REQUEST du site n° %d\n",myID,message_buffer.idEnvoyeur);
                        if (*last == NIL_PROCESS){
                            if (*requete_SC == TRUE) {
                                *next = message_buffer.idDemandeurSC;
                            }
                            else {
                                printf("Site n° %d envoit le TOKEN à son next le site n° %d\n",myID,message_buffer.idDemandeurSC);
                                envoiMessage(myID,TOKEN,message_buffer.idDemandeurSC,NIL_PROCESS);
                            }
                        }
                        else {
                            printf("Site n° %d fait passer la REQUEST à son last le site n° %d\n",myID,*last);
                            envoiMessage(myID,REQUEST,*last,message_buffer.idDemandeurSC); 
                        }
                        *last = message_buffer.idDemandeurSC;
                        pthread_mutex_unlock(adrMutex);
                        break;
                    case QUIT:
                        pthread_mutex_lock(adrMutex);
                        printf("Site n° %d à recut QUIT du site n° %d\n",myID,message_buffer.idEnvoyeur);
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
                //perror("Recv()");

        } while( res > 0 );
    }
    printf("Thread terminé n°%d,  %d",myID,*nbSitesTermine);
    pthread_exit(NULL); 
}

void * travail(void * params){
    threadArgs *arg = (struct threadArgs *)params;
    //printf("Thread Travail n°%d\n",arg->myID);

    int nb_entree_SC = 0;
    int myID = arg->myID; 
    int * requete_SC = arg->requete_SC; 
    int * next = arg->next; 
    int * last = arg->last;
    int * token_present = arg->token_present; 
    pthread_mutex_t * mutex = arg->mutex;

    int c;
    printf( "Enter a value if all site has be launched\n");
    c = getchar();         
   
    for (nb_entree_SC = 0;nb_entree_SC< NB_EXEC_SC;nb_entree_SC++){
        desireRentrerenSC(myID, requete_SC,next, last, token_present, mutex);
        
        printf("Site n°%d entre en SC pour la %d ième fois\n",myID,nb_entree_SC+1);
        calcul(1); //SC
        printf("Site n°%d sort de la SC\n",myID);
        desireSortirSC(myID, requete_SC,next, last, token_present, mutex);
    }

    for (int i = 0; i <NB_NODES;i++) 
    {
        if (i != myID) {
            envoiMessage(myID,QUIT,i,NIL_PROCESS);
            printf("Site n° %d envoit QUIT au site n° %d\n",myID,i);
        }
    }
    pthread_exit(NULL); 
}


void run(void){
    int last = 0;
    int next = NIL_PROCESS; 
    int requete_SC = FALSE; 
    int token_present = FALSE; 
    int nbProcessTermines = 0;
    
    if (myID == 0){
        last = NIL_PROCESS;
        token_present = TRUE;
    }

    pthread_t thread_receveur;
    pthread_t thread_travailleur;
    pthread_mutex_t thread_mutex;

    thread_mutex = PTHREAD_MUTEX_INITIALIZER;

    threadArgs argReceveur;

    argReceveur = {myID,start_port + myID,&last,&next,&requete_SC,&token_present,&nbProcessTermines,&thread_mutex};

    pthread_create(&thread_receveur,NULL,traitement_message,&argReceveur);

    threadArgs argTravailleur;

    argTravailleur = {myID,start_port + myID,&last,&next,&requete_SC,&token_present,&nbProcessTermines,&thread_mutex};

    pthread_create(&thread_travailleur,NULL,travail,&argTravailleur);

    pthread_join(thread_receveur,NULL);
    pthread_join(thread_travailleur,NULL);

    pthread_mutex_destroy(&thread_mutex);
}


int desireRentrerenSC(int myID, int * requete_SC, int * next, int * last, int * has_token, pthread_mutex_t *verrou)
{
    //printf("Site n°%d desireRentreSC son next est%d\n",myID,*next);
	pthread_mutex_lock(verrou);
	*requete_SC = TRUE;
	if(*last!=-1) // si j'ai un père
	{
		if (envoiMessage(myID,REQUEST,*last,myID) == FALSE) {
            printf("Erreur sur l'envoi de request du site %d au site %d",myID,*last);
            exit(EXIT_FAILURE);
        }
        printf("Site n° %d envoit REQUEST au site n° %d\n",myID,*last);
		*last = NIL_PROCESS; // je deviens racine de l'anti arborescence
	}
    
    pthread_mutex_unlock(verrou);

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
    if (*next !=NIL_PROCESS){ // j'ai un next
        if (envoiMessage(myID, TOKEN, *next, NIL_PROCESS) == FALSE)
        {
            printf("Erreur sur l'envoi de jeton du site %d au site %d",myID,*next);
            exit(EXIT_FAILURE);
        }
        printf("Site n° %d envoit TOKEN au site n° %d\n",myID,*next);
        *has_token = FALSE; 
        *next = NIL_PROCESS;
    }
	
    pthread_mutex_unlock(verrou);
    return 0;
}

int envoiMessage(int myID, enum messageType type, int destiID, int idDemandeurSC){

    /* Avant de poursuivre dans la fonction, il est important de s'assurer
    que le site émetteur ne soit pas également le site récépteur */
    if (myID == destiID){
        printf("Le site envoyant le message est le même que celui recevant le message. Arrêt du programme\n.");
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

    //printf("message envoyé du site:%d {envoyeur: %d, receveur: %d, type: %d, demandeurSc: %d\n",myID,message.idEnvoyeur, message.idReceveur,message.type,message.idDemandeurSC);

    close(sockEnvoi);
    return TRUE;
}
