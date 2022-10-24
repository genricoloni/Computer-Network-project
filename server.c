#include "utils.c"

int main(int argc, char* argv[]){
    uint32_t listener, communicate, ret, addrlen, fdmax, i;

    fd_set master, readfds;

    struct sockaddr_in server_addr, client_addr;

    int port;
    port = findPort(argc, argv);

<<<<<<< HEAD
    printf("<<<<<<<<<<<<<<SERVER ONLINE SULLA PORTA  %d>>>>>>>>>>>>>> \n", port);

    
=======
    printf("La porta selezionata è %d \n", port);
>>>>>>> e7c7997e51e8f8ea31c6de959ecb44b6615cfcd0

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
                        
                        
                        int code, res;
                        uint32_t code_t, res_t;
                        printf("codice con lo switch, controlla anche la disconnessione\n");
                        ret = recv(i, (void*)&code_t, sizeof(uint32_t), 0);
                        printf("dopo recv\n");
                        if(ret == 0){
                            printf("dopo recv con ret == 0");/*
                            // il client ha chiuso il socket, quindi
                            // chiudo il socket connesso sul server
                            close(i);

                            // rimuovo il descrittore newfd da quelli da monitorare
                            FD_CLR(i, &master);

                            printf("CHIUSURA client %d rilevata!\n", i);
                            fflush(stdout);
                
                            continue;*/
                        }
                    
                     
                        code = ntohl(code_t);
                        printf("%d\n", code);
                        printf("prima dello switch\n");
                        
                        switch (code)
                        {
                        case SIGNUP_CODE:
                            //codice riconosciuto: invio primo ack
                            printf("debug signup\n");
<<<<<<< HEAD
                            fflush(stdout);
=======
>>>>>>> e7c7997e51e8f8ea31c6de959ecb44b6615cfcd0
                            //res = ACK;
                            //res_t = htonl(res);
                            //ret = send(i, (void*)res_t, sizeof(uint32_t), 0);
                            signup_s(i);
<<<<<<< HEAD
                            fflush(stdout);
                            printf("DAIIII PORCODDIOOOOOOOOOOO\n");
                            fflush(stdout);
=======
                            fflush(stdin);
>>>>>>> e7c7997e51e8f8ea31c6de959ecb44b6615cfcd0
                            printf("dopo signup\n");
                            
                            break;
                        case IN_CODE:
                            printf("debug in");
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











}
