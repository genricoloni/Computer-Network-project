#include "utils.c"

int DEVICE_PORT;
char OWN_USER[USERN_CHAR];

int main(int argc, char* argv[]){
    
    char command[MAXCMD], port[MAXPORT];
    char buffer[BUFSIZE], tmpbuff[BUFSIZE+3];
    char username[USERN_CHAR], mittente[USERN_CHAR], *filename = NULL;
    int code, cl_socket, codeN, chat_code;
    uint32_t server_com, cl_listener, ret, fdmax, i;
    uint32_t addrlen, code_t;
    char path[100];

    struct credenziali credenziali;

    bool connected = false, conn_error = false, cmd_err = false, su = true, reg = false, in = false, login = true;
    bool client_offline = false, in_chat = false, in_group = false;

    // strutture per indirizzi
    struct sockaddr_in server_addr, client_addr, cl_listener_addr, gp_addr;

    fd_set master, readfds;

    system("clear");

	FD_ZERO(&master);
	FD_ZERO(&readfds);

    OWN_PORT = findPort(argc, argv);

    //pulizia memoria e gestione indirizzi del SERVER
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    //socket col server

    server_com = socket(AF_INET, SOCK_STREAM, 0);

    //aggiungp il socket appena creato alla lista dei socket da monitorare
    FD_SET(STDIN, &master);

	

    //pulizia memoria e gestione indirizzi del CLIENT
	memset(&client_addr, 0, sizeof(client_addr));
	cl_listener_addr.sin_family = AF_INET;
	cl_listener_addr.sin_port = htons(atoi(argv[1]));
	inet_pton(AF_INET, "127.0.0.1", &cl_listener_addr.sin_addr);

	cl_listener = socket(AF_INET, SOCK_STREAM, 0);
	bind(cl_listener, (struct sockaddr *)&cl_listener_addr, sizeof(cl_listener_addr));
	listen(cl_listener, 50);

	FD_SET(cl_listener, &master);
	fdmax = cl_listener;

 

   
   /*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<MENU DI LOGIN/SIGNUP>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
    
    while (1)
    {   
        printf("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<AREA DI ACCESSO>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
        printf("1)signup port user password -->per registrarsi al servizio\n");
        printf("2)in     port user password -->per  accedere   al servizio\n");

        if(conn_error == true)
            perror("Errore in fase di connessione col server, verificare la porta;\n");

        if(cmd_err == true)
            printf("+++Comando [%s] non riconosciuto+++\n", command);

        if(reg == true)
            printf("Utente già registrato!\n");

        if(su == false)
            printf("credenziali registrate correttmente!\n");

        if(login == false)
            printf("credenziali di accesso errate o utente non registrato!\n");


        conn_error = false;
        cmd_err = false;
        reg = false;
        fgets(buffer, 1024 - 1, stdin);
		sscanf(buffer, "%s %s %s %s", command, port, credenziali.username, credenziali.password);
		
        code = cmd_to_code(command);

        if(code == -1){
            cmd_err = true;
            system("clear");
            continue;
        }

        if(connected == false){
            //mi connetto al server usando la porta inserita in input
            server_addr.sin_port = htons(atoi(port));

            ret = connect(server_com, (struct sockaddr*)&server_addr, sizeof(server_addr));

            if (ret < 0){
                conn_error = true;
                system("clear");
                continue;            
            }
            
            connected = true;
            printf("<<<<<<<<<<<<<<<<<<<<<<<<CONNESSIONE RIUSCITA>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
        }
       
        switch (code){
            case SIGNUP_CODE: 
                reg = signup_c(code, credenziali, server_com);
                su = reg;
                //system("clear");
                break;

            case IN_CODE:
                in = login_c(code, credenziali, server_com, atoi(argv[1]));
                login = in;
                system("clear");
                break;

            default:
                cmd_err = true;
                system("clear");
                break;
    }   
    if(in == true){
        strcpy(OWN_USER, credenziali.username);
        break;  
    } 
    }

/*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<MENU PRINCIPALE>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
    print_menu(OWN_USER);

    fflush(stdin);

    code = -4;
    client_offline = true;

    while(1){
        readfds = master;
        select(fdmax + 1, &readfds, NULL, NULL, NULL);

        for(i = 0; i <= fdmax; i++){

            if(FD_ISSET(i, &readfds)){
                        fflush(stdin);

                if(i == STDIN){

                    if (in_chat == true){
                        struct destinatari *tmp = destinatari;

                        //printf("Debug: in chat\n");

                        //prendo in input il messaggio da inviare
                        fgets(buffer, 1024 - 1, stdin);
                        buffer[strlen(buffer) - 1] = '\0';

                        //controllo se il messaggio è un comando
                        if(strcmp(buffer, "/q\0") == 0){
                            in_chat = false;
                            //chiudo il socket e rimuovo dai destinatari se sono in chat P2P;
                            if(client_offline == true){
                                rimuovi_tutti_destinatari();
                                }
                            client_offline = true;
                            
                            system("clear");
                            print_menu(OWN_USER);
                            continue;
                        }

                        if(client_offline == false && chat_code == 0){
                            //devo inviare il messaggio al server
                            fflush(stdin);
                            uint32_t code_t = htonl(MSG_CODE);

                            strcpy(tmpbuff, "* : ");
                            strcat(tmpbuff, buffer);


                            //chiamo una funzione che stampa il contenuto di un file
                            append_msg_c( tmpbuff, destinatari->username, OWN_USER);
                            system("clear");
                            print_chat(OWN_USER, destinatari->username);
                            memset(tmpbuff, 0, sizeof(tmpbuff));
                            send(server_com, &code_t, sizeof(uint32_t), 0);



                            send(server_com, &destinatari->username, sizeof(destinatari->username), 0);
                            send(server_com, &buffer, sizeof(buffer), 0);
                            //system("clear");
                            //print_chat(path);



                            //printf("CHAT CON %s\n", destinatari);
                            fflush(stdin);
                            continue;
                        }

                        if(client_offline == true && chat_code > 0){
                            int a;
                            //devo inviare il messaggio al client

                            //invio il messaggio a tutti i destinatari
                            while(tmp != NULL){
                                struct sent_message *msg = malloc(sizeof(struct sent_message));
                                int ack;

                                strcat(buffer, "   ");
                                strcpy(tmpbuff, "**: ");
                                
                                uint32_t code_t = htonl(MSG_CODE);

                                
                                //invio codice
                                a = send(tmp->socket , &code_t, sizeof(uint32_t), 0);

                                a = recv(tmp->socket , &ack, sizeof(uint32_t), 0);

                                if (a <= 0){
                                    client_offline = false;
                                    if (in_group == false){
                                        printf("Client disconnesso inaspettatatamente: rinviare il messaggio\n");

                                        //non sono in una chat di gruppo
                                        //rimuovo il destinatario dalla lista
                                        //e aggiungo il server alla lista
                                        inserisci_destinatario(tmp->username, server_com);
                                        rimuovi_destinatario(tmp->username);
                                        client_offline = false;
                                        chat_code = 0;
                                        break;

                                    }
                                    else{
                                        //sono in una chat di gruppo
                                        //rimuovo il destinatario dalla lista
                                        rimuovi_destinatario(tmp->username);
                                        printf("Sembra che il client %s sia disconnesso, non riceverà più i messaggi da questa chat di gruppo\n", tmp->username);
                                        break;
                                    }
                                }

                                a = send(tmp->socket , OWN_USER, USERN_CHAR, 0);
                                //ricevo ack

                                a = recv(tmp->socket , &ack, sizeof(uint32_t), 0);


                                a = send(tmp->socket , buffer, BUFSIZE, 0);

                                strcpy(tmpbuff, "**: ");
                                strcat(tmpbuff, buffer);
                                sprintf(path, "./%s/chat/%s.txt", OWN_USER, tmp->username);
                                append_msg_c( tmpbuff, tmp->username, OWN_USER);
                                system("clear");
                                print_chat(OWN_USER, tmp->username);
                                free(msg);
                                // scorro la lista dei destinatari
                                tmp = tmp->next;                        }
                                continue;
                        }
                    }   

                        fflush(stdin);

                    //è un comando dallo stdin
                    memset(&buffer, 0, sizeof(buffer));
                    memset(&command, 0, sizeof(command));
                    memset(&username, 0, sizeof(username));
                    fgets(buffer, 1024 - 1, stdin);
                    sscanf(buffer, "%s %s", command, username);
                    code = cmd_to_code(command);
                    
                    //switch case con tutti i casi per i diversi comandi
                    switch (code){
                        case HANG_CODE:
                            hanging_c(code, server_com);
                            
                            break;

                        case SHOW_CODE:
                            //prelevo username dal buffer
                            //sscanf(buffer, "%s %s", command, username);
                            show_c(code, username, server_com, OWN_USER);
                            wait();
                            system("clear");
                            print_menu(OWN_USER);
                            break;

                        case CHAT_CODE:
                            chat_code = chat_init_c(code, username, server_com);
                            //gestione struct e memoria
                            fflush(stdin);

                            if (chat_code == -1){
                                client_offline = false;
                                printf("L'utente non esiste\n");
                                break;
                            }
                            if (chat_code == 0){
                                client_offline = false;
                                //system("clear");
                                //sprintf(path, "./%s/chat/%s", OWN_USER, destinatari->username);
                                sprintf(path, "./%s/chat/%s.txt", OWN_USER, destinatari->username);

                                //print_chat(path);
                                //printf("Dopo print_chat\n");
                                in_chat = true;
                                system("clear");
                                printf("path: %s\n", path);

                                print_chat(OWN_USER, destinatari->username);

                                break;
                            }
                            printf("CHAT CON %s\n", username);
                            memset(&client_addr, 0, sizeof(client_addr));
                            client_addr.sin_family = AF_INET;
                            client_addr.sin_port = htons(chat_code);
                            inet_pton(AF_INET, "127.0.0.1", &client_addr.sin_addr);
                            cl_socket = socket(AF_INET, SOCK_STREAM, 0);

                            inserisci_destinatario(username, cl_socket);

                            sprintf(path, "./%s/chat/%s.txt", OWN_USER, destinatari->username);

                            ret = connect(cl_socket, (struct sockaddr*)&client_addr, sizeof(client_addr));
                            in_chat = true;

                            system("clear");
                            printf("path: %s\n", path);

                            print_chat(OWN_USER, destinatari->username);
                            break;

                        case SHARE_CODE:
                            //prelevo username e filename dal buffer
                            sscanf(buffer, "%s %s %s", command, username, filename);
                            //share_c(code, server_com, buffer);
                            break;

                        case OUT_CODE:
                            //il client notifica al server la disconnessione
                            //out_c(code, server_com);
                            exit(0);
                            break;

                        default:
                            printf("+++Comando [%s] non riconosciuto+++\n", command);
                            break;
                    }

                    

                
                    
                } else{
                    if(i == cl_listener){
                        //è una nuova connessione
                        addrlen = sizeof(gp_addr);
                        ret = accept(i, (struct sockaddr*)&gp_addr, (socklen_t*)&addrlen);

                        fdmax = (ret > fdmax) ? ret : fdmax;

                        FD_SET(ret, &master);
                        continue;

                        
                    } else{
                        //è un messaggio da un client già connesso   

                        memset(&buffer, 0, sizeof(buffer));
                        ret = recv(i, mittente, USERN_CHAR, 0);
                        codeN = ntohl(code_t);
                        send(i, &code_t, sizeof(uint32_t), 0);

                        if(ret <= 0){
                            if(ret == 0){
                                printf("Connessione chiusa da %s:%d\n", inet_ntoa(cl_listener_addr.sin_addr), ntohs(cl_listener_addr.sin_port));

                            } else{
                                perror("Errore in fase di ricezione del messaggio");
                            }
                            close(i);
                            FD_CLR(i, &master);
                        } else{
                            code_t = htonl(codeN);
                            //printf("è un messaggio da un client già connesso\n");
                            //ricevo messaggio da un client
                            recv(i, mittente, USERN_CHAR, 0);

                            send(i, &code_t, sizeof(uint32_t), 0);


                            recv(i, buffer, BUFSIZE, 0);
                            
                            //funzione che scrive nel file di chat il messaggio ricevuto
                            if (in_group == false){
                            
                                append_msg_rcv(mittente, buffer, OWN_USER);
                            }
                            if (in_chat == true){
                                system("clear");
                                print_chat(OWN_USER, mittente);
                            }
                        }
                        continue;
                    }
                    continue;
                }
            }
        }
    }
}