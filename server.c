#include "utils.c"

int main(int argc, char* argv[]){
    // variabili per socket, descrittoremax
    uint32_t listener, comm, ret, addrlen, fdmax;    
    
    // strutture per indirizzi
    struct sockaddr_in server_addr, client_addr;

    //lista di descrittori
    fd_set master, readfds;

    int port = findPort(argc, argv);

    char buffer[4096];
    
    //pulizia memoria e gestione indirizzi
    memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    system("clear");
    printf("INIZIALIZZAZIONE SOCKET...\n");
    
	// socket di listen
	listener = socket(AF_INET, SOCK_STREAM, 0);

	// aggancio del listener
	bind(listener, (struct sockaddr *)&server_addr, sizeof(server_addr));
	listen(listener, 50);
    printf("<<<<<<<<<<<<<<<<<<<<<<<<<AGGANCIO EFFETTUATO>>>>>>>>>>>>>>>>>>>>>>>>>\n");

    //lista dei socket da monitorare
	FD_SET(listener, &master);
	fdmax = listener;

    while (1){
        readfds = master;

        select(fdmax+1, &readfds, NULL, NULL, NULL);
        uint32_t i;
        for(i = 0; i<= fdmax; i++){
            if(FD_ISSET(i, &readfds)){
                if(i == STDIN){
                    char cmd[9];
                    scanf("%s", cmd);
                    //menu dei comandi
                } else{
                    if(i == listener){
                        //socket di lettura
                        int addrlen = sizeof(client_addr);

                        comm = accept(i, (struct sockaddr*)&client_addr, (socklen_t*)&addrlen);
                       
                       fdmax = (fdmax <= comm)? fdmax : comm;
                       FD_SET(comm, &master);
                       printf("NUOVA CONNESSIONE\n");
                        

                    }
                }
            }
        }

    }
    
    
    return 0;
}