#ifndef NB_NODES
#define NB_NODES 3
#define NIL_PROCESS -1
#define NIL -1
#define TRUE 1
#define FALSE 0
#define NB_EXEC_SC 4

enum messageType {
    TOKEN,
    REQUEST,
    QUIT
};

struct message {
    enum messageType type;
    int idDemandeurSC;
    int idEnvoyeur;
    int idReceveur;
};

struct threadArgs {
    int myID;
    int myPort;
    int * last;
    int * next;
    int * requete_SC;
    int * token_present;
    int * nbProcessTermines;
    FILE * file;
    pthread_mutex_t * mutex;
    /* other parameters here ... */
};

int creeSocketReceveur(int myID,int myPort);
int envoiMessage(int myId, enum messageType ,int destiId);
int envoiMessage(int myId, enum messageType ,int destiId,int idDemandeurCS);
int desireSortirSC(int myID, int * requete_SC, int * next, int * last, int * has_token, pthread_mutex_t *verrou);
int desireRentrerenSC(int myID, int * requete_SC, int * next, int * last, int * has_token, pthread_mutex_t *verrou);
void * travail(void * s);
void * traitement_message(void * s);
void run(int nodeInitialJeton, FILE *file, int start_port);

#endif