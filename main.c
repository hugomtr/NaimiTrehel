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
int MAX_SIZE = 1000;
int LENGTH_MAX = 3000;
int neighbors [3000]; 
int root_port;

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
        int firstSocketconnexion = accept(*ptrmainsocket,(struct sockaddr *)&addrNeighborsAccepted, &t);
        if (firstSocketconnexion>0){
            printf("connexion accepté par le socket principal");
            char buffer[MAX_SIZE];
            recv(firstSocketconnexion,buffer,sizeof(buffer),0);// receive data from the socket connected
        }   

    }
}

void * connectneighbors(void* general_port)
{
    int* port = (int *)general_port;
    for (int curr_port = *port; curr_port<= *port + n_site; curr_port++)
    {
        if (neighbors[curr_port] == -1 && curr_port != root_port)
        {
            neighbors[curr_port] = socket(AF_INET, SOCK_STREAM, 0);
            
            if (neighbors[curr_port] == -1){
                printf("Client : pb creation socket\n");
                exit(1);
            }

            printf("Création de la socket\n");
            
            struct sockaddr_in adrServ;
            adrServ.sin_family = AF_INET;
            adrServ.sin_addr.s_addr = inet_addr("127.0.0.1");
            adrServ.sin_port = htons(curr_port);

            int lgAdr = sizeof(struct sockaddr_in);

            int conn = connect(neighbors[curr_port], (struct sockaddr *)&adrServ,sizeof(adrServ));

            if (conn < 0) 
            {
                neighbors[curr_port] = -1;
            } else {
                char buffer[MAX_SIZE];
                send(conn,)
            }

        }
    }
}


int main(int argc, char *argv[])
{
    if (argc<2){
        printf("utilisation: %s numero_port_main_socket entre 2000 et 2900 \n", argv[0]);
        exit(1);
    }

    int general_port = atoi(argv[1]);
    
    int general_sock = connexion(general_port);

    int neighbors[general_port + n_site];
    for (int i = 0; i < LENGTH_MAX; i++){
        neighbors[i] = -1;
    }

    pthread_t acceptConnexion;
    pthread_create(&acceptConnexion,NULL,acceptneighbors,&general_sock);

    pthread_t connectAllSite;
    pthread_create(&connectAllSite,NULL,connectneighbors,&general_port);

    


    printf("Serveur : fin du dialogue avec les utilisateurs\n");
	for(int i=general_port; i<general_port + n_site; i++) {
		if(neighbors[i]!=-1 && i!=general_port) {
			close( neighbors[i] );
		}
	}

    close(general_sock);
    return 0;
}


