#ifndef NB_NODES
#define NB_NODES 5
#define NIL_PROCESS -1

enum messageType {
    TOKEN,
    REQUEST,
};

struct message {
    enum messageType type;
    int idEnvoyeur;
    int idRequest;
};

struct threadArgs {
    int myID;
    int myPort;
    int * last;
    int * next;
    int * requete_CS;
    int * token_present;
    /* .... */
};

int creeSocketReceveur(int myID,int myPort);
bool envoiMessage(int myId, int destiId);
bool desireSortirSC(int myID, int &requete_CS, int &next, int &last, int &has_token, pthread_mutex_t *m_sendrec);
bool desireRentrerenSC(int myID, int &requete_CS, int &next, int &last, int &has_token, pthread_mutex_t *m_sendrec);
void * travail(void * s);
void * traitement_message(void * s);
void run(int nodeInitialJeton, FILE file, int start_port);

#endif