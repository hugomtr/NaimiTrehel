#ifndef NB_NODES
#define NB_NODES 5
#define NIL_PROCESS -1
#define NIL -1
#define TRUE 1
#define FALSE 0


enum messageType {
    TOKEN,
    REQUEST,
    QUIT
};

struct message {
    enum messageType type;
    int idEnvoyeur;
    int idReceveur;
};

struct threadArgs {
    int myID;
    int myPort;
    int nb_entree_SC;
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
int desireSortirSC(int myID, int * requete_SC, int * next, int * last, int * has_token, pthread_mutex_t *verrou);
int desireRentrerenSC(int myID, int * requete_SC, int * next, int * last, int * has_token, pthread_mutex_t *verrou);
void * travail(void * s);
void * traitement_message(void * s);
void run(int nodeInitialJeton, FILE *file, int start_port);

#endif