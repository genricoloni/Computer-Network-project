#include "utils.c"

int main(int argc, char* argv[]){
    uint32_t listener, communicate, ret, addrlen, fdmax, i;

    fd_set master, readfds;

    struct sockaddr_in server_addr, client_addr;

    int port;
    port = findPort(argc, argv);

    printf("<<<<<<<<<<<<<<SERVER ONLINE SULLA PORTA  %d>>>>>>>>>>>>>> \n", port);

    

    //char buffer[4096];
    FD_ZERO(&master);
	FD_ZERO(&readfds);
	FD_SET(0, &master);

    //pulizia memoria e gestione indirizzi
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    listener = socket(AF_INET, SOCK_STREAM, 0);

	bind(listener, (struct sockaddr *)&server_addr, sizeof(server_addr));
	listen(listener, 50);

	FD_SET(listener, &master);
	fdmax = listener;

    for(;;){

        readfds = master;
        fflush(stdin);
        select(fdmax + 1, &readfds, NULL, NULL, NULL);

        for ( i = 0; i <= fdmax; i++){

            if(FD_ISSET(i, &readfds)){

                if(i == STDIN){
                    char Command[100];
                    fscanf(stdin, "%s", Command);
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
                } else{
                    if(i == listener){
                        addrlen = sizeof(client_addr);
                        communicate = accept(i, (struct sockaddr*)&client_addr, (socklen_t*)&addrlen);

                        fdmax = (communicate > fdmax) ? communicate : fdmax;

                        FD_SET(communicate, &master);

                        printf("Nuova connessione \n");
                    }
                    else{
                        

                        int code;
                        uint32_t code_t;
                        ret = recv(i, (void*)&code_t, sizeof(uint32_t), 0);;
                        if(ret == 0){
                            printf("dopo recv con ret == 0");
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

                        
                        switch (code)
                        {
                        case SIGNUP_CODE:
                            //codice riconosciuto
                            signup_s(i);
                            
                            break;
                        case IN_CODE:
                            printf("debug in");
                            break;

                        default:
                            break;
                        }
                        system("clear");
                        
                    }
                }

            }
        }

    }











}
