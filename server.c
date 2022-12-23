#include "utils.c"


int main(int argc, char* argv[]){
    uint32_t listener, communicate, ret, addrlen, fdmax, i;

    fd_set master, readfds;

    struct sockaddr_in server_addr, client_addr;
    

    int port;
    int code;
    port = findPort(argc, argv);
    bool res;

    system("clear");
    printf("<<<<<<<<<<<<<<SERVER ONLINE SULLA PORTA  %d>>>>>>>>>>>>>> \n", port);

    stampa_comandi_server();

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
                    code = codifica_comando_server(Command);
                    switch (code)
                    {
                        case LIST_CODE:
                            fflush(stdin);
                            printf("LISTA UTENTI ONLINE:\n");
                            stampa_lista_utenti_online(utenti_online);
                            getchar();
                            system("clear");
                            stampa_comandi_server();
                            break;
                        
                        case HELP_CODE:
                            stampa_help_server();
                            getchar();  
                            system("clear");
                            stampa_comandi_server();                              
                            break;
                        
                        case ESC_CODE:
                            printf("SERVER CHIUSO\n");
                            exit(0);
                            break;

                        default:
                            printf("Comando non riconosciuto\n");
                            break;
                    }
                    
                } else{
                    
                    if(i == listener){
                        //nuova connessione
                        addrlen = sizeof(client_addr);
                        communicate = accept(listener, (struct sockaddr*)&client_addr, (socklen_t*)&addrlen);

                        fdmax = (communicate > fdmax) ? communicate : fdmax;

                        FD_SET(communicate, &master);

                        printf("Nuova connessione \n");
                    }
                    else{
                        

                        int code;
                        uint32_t code_t;
                        ret = recv(i, (void*)&code_t, sizeof(uint32_t), 0);
                        if(ret == 0){
                            
                            // il client ha chiuso il socket, quindi
                            // chiudo il socket connesso sul server
                            out_s(get_username(i));
                            close(i);
                            rimuovi_utente(&utenti_online, i);


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
                            printf("signup completata\n");
                            //system("clear");
                            
                            break;
                        case IN_CODE:
                            
                            fflush(stdout);
                            res = login_s(i, &utenti_online);
                            if(res == true)
                                printf("UTENTE  %s ONLINE\n", utenti_online->username);
                            break;

                        case HANG_CODE:
                            hanging_s(i);
                            break;

                        case SHOW_CODE:
                            printf("Debug: SHOW_CODE\n");
                            show_s(i);
                            break;
                        
                        case CHAT_CODE:
                            chat_s(i);
                            break;

                        case MSG_CODE:
                            //caso in cui il server riceve un messaggio che
                            //diventerà pendente
                            msg_s(i);
                            break;
                        case ADD_CODE:
                            add_s(i);
                            break;
                        

                        default:
                            break;
                        }
                        //system("clear");
                        
                    }
                }

            }
        }

    }











}
