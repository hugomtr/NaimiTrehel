#include <stdio.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

/// VA globales
int n_site = 3;


int connexion(int port){
    int ds = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in ad;
    ad.sin_family = AF_INET;
    ad.sin_addr.s_addr = INADDR_ANY;
    ad.sin_port = htons(port);

    if(bind(ds,(struct sockaddr *)&ad,sizeof(ad)) < 0){
        perror("Serveur : erreur bind");
        close(ds); 
        exit(1);
    }

    printf("Serveur: nommage : ok\n");

    int ecoute = listen(ds,5);
    if (ecoute < 0){
        printf("Serveur : je suis sourd(e)\n");
        close (ds);
        exit (1);
    } 

    printf("Serveur: mise en écoute : ok\n");
}

void * acceptneighbors(void* socket){
    int* ptrmainsocket = (int *) socket;
    struct sockaddr_in addrNeighborsAccepted;
    socklen_t t = sizeof (struct sockaddr_in);
    while(1)
    {
        int firstSocket = accept(*ptrmainsocket,(struct sockaddr *)addrNeighborsAccepted, &t);    
        
    }
}

void * connectneighbors(void* s)
{
    
}


int main(int argc, char *argv[])
{
    if (argc<2){
        printf("utilisation: %s numero_port_main_socket \n", argv[0]);
        exit(1);
    }

    int general_port = atoi(argv[1]);
    
    int general_sock = connexion(general_port);

    int neighbors[general_port + n_site];
    for (int i = 0; i < n_site; i++){
        neighbors[i] = -1;
    }


    pthread_t acceptConnexion;
    pthread_create(&acceptConnexion,NULL,acceptneighbors,&general_sock);

    pthread_t connectAllSite;
    pthread_create(&connectAllSite,NULL,connectneighbors, NULL);


    return 0;
}


