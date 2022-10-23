#include "utils.c"

int main(int argc, char* argv[]){
    // variabili per socket, descrittoremax
    uint32_t listener, ret, addrlen, fdmax, communicate, i;    
    
    // strutture per indirizzi
    struct sockaddr_in server_addr, client_addr;

    //lista di descrittori
    fd_set master, readfds;

    int port;
    port = findPort(argc, argv);

    printf("La porta selezionata è %d \n", port);

    //char buffer[4096];
    FD_ZERO(&master);
	FD_ZERO(&readfds);
	FD_SET(0, &master);

    
    //pulizia memoria e gestione indirizzi
    memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    system("clear");
    printf("INIZIALIZZAZIONE SOCKET...\n");
    printf("La porta selezionata è %d \n", port);

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
        fflush(stdin);
        printf("dentro while\n");
        select(fdmax+1, &readfds, NULL, NULL, NULL);
        
        for( i = 0; i <= fdmax; i++){
            printf("dentro for\n");

            if(FD_ISSET(i, &readfds)){
                printf("primo if");

                if(i == STDIN){
					char Command[100];
                    printf("comando\n");
					// Check della correttezza del comando sullo stdin
					fscanf(stdin, "%s", Command);
                    
					// ------------------ switching comandi server -------------------
					if (strcmp(Command, "help") == 0)
					{
						// comando help
						//stampa_comandi_server();
					}
					else if (strcmp(Command, "list") == 0)  
					{
						// comando list
						//comando_list();
					}
					else if (strcmp(Command, "esc") == 0)
					{
						// comando esc
						exit(1);
					}
					else
					{
						// comando non valido, torno sopra
						printf("comando non esistente\n");
						continue;  
                    }                               
                } 
                else{
                    printf("else del primo if");
                    if(i == listener){
                        printf("tento accept");
                        //accetto le richieste di connessione
                        addrlen = sizeof(client_addr);
                        
                        communicate = accept(i, (struct sockaddr*)&client_addr, (socklen_t *)&addrlen);
                        fdmax = (communicate > fdmax) ? communicate : fdmax;
                        FD_SET(communicate , &master);
                        
                       printf("NUOVA CONNESSIONE\n");
                    }
                    else{
                        printf("ELSE SECONDO IF");
                        //GESTIONE DI UNA RICHIESTA
                        // se non è il listener, 'i'' è un descrittore di socket
                        // connesso di comunicazione che ha fatto la richiesta, e va servito
                        // ***senza poi chiudere il socket***
                        int code, res;
                        uint32_t code_t, res_t;
                        ret = recv(i, (void*)&code_t, sizeof(uint32_t), 0);
                        printf("dopo recv");
                        if(ret <= 0){
                            // il client ha chiuso il socket, quindi
                            // chiudo il socket connesso sul server
                            close(i);

                            // rimuovo il descrittore newfd da quelli da monitorare
                            FD_CLR(i, &master);

                            printf("CHIUSURA client %d rilevata!\n", i);
                            fflush(stdout);
                
                            continue;
                        }
                    
                     
                        code = ntohl(code_t);

                        printf("prima dello switch");
                        switch (code)
                        {
                        case SIGNUP_CODE:{
                            //codice riconosciuto: invio primo ack
                            printf("debug");
                            //res = ACK;
                            //res_t = htonl(res);
                            //ret = send(i, (void*)res_t, sizeof(uint32_t), 0);
                            signup_s(i);
                            }
                            break;
                        case IN_CODE:
                            break;
                            default:
                            printf("Impossibile gestire la richiesta: codice non riconosciuto");
                            res = ERR_CODE;
                            res_t = htonl(res);
                            ret = send(i, (void*)&res_t, sizeof(uint32_t), 0);
                            break;
                        }
                    }
                }
            }
        }

    }
    printf("CHIUDO IL LISTENER!\n");
    fflush(stdout);
    close(listener);
}
