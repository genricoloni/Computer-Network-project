#include "utils.c"

int DEVICE_PORT;

int main(int argc, char* argv[]){
    
    char command[MAXCMD], port[MAXPORT];
    char buffer[BUFSIZE];
    char* username, *filename;
    int code, i;
    int server_com, cl_listener, ret;
    int fdmax = 0;

    struct credenziali credenziali;

    bool connected = false, conn_error = false, cmd_err = false, su = true, reg = false, in = false;

    // strutture per indirizzi
    struct sockaddr_in server_addr, client_addr, client_listener_addr;

    fd_set master, readfds;

    system("clear");

	FD_ZERO(&master);
	FD_ZERO(&readfds);

    DEVICE_PORT = findPort(argc, argv);

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
    client_listener_addr.sin_family = AF_INET;
    client_listener_addr.sin_port = htons(atoi(argv[1]));
    inet_pton(AF_INET, "127.0.0.1", &client_listener_addr.sin_addr);

    cl_listener = socket(AF_INET, SOCK_STREAM, 0);
    bind(cl_listener, (struct sockaddr*)&client_listener_addr, sizeof(client_listener_addr));
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
                system("clear");
                break;

            case IN_CODE:
                in = login_c(code, credenziali, server_com, atoi(argv[1]));
                system("clear");
                break;

            default:
                cmd_err = true;
                system("clear");
                break;
    }   
    if(in == true)
        break;
    }

/*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<MENU PRINCIPALE>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
        printf("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<Menu principale>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
        printf("1)hanging                   -->ricevi i messaggi pendenti quando eri offline\n");
        printf("2)show  username            -->mostra i messaggi inviati da username\n");
        printf("3)chat  username            -->avvia una chat con username\n");
        printf("4)share username file_name  -->condividi il file file_name con tutti gli utenti connessi\n");
        printf("5)out                       -->disconnetti l'utente\n");


    while(1){
        readfds = master;
        select(fdmax + 1, &readfds, NULL, NULL, NULL);

        for(i = 0; i <= fdmax; i++){

            if(FD_ISSET(i, &readfds)){

                if ( i == STDIN){
                    //è un comando dallo stdin
                    memset(&buffer, 0, sizeof(buffer));
                    memset(&command, 0, sizeof(command));
                    fgets(buffer, 1024 - 1, stdin);
		            sscanf(buffer, "%s", command);

                    code = cmd_to_code(command);
                    
                    //switch case con tutti i casi per i diversi comandi
                    switch (code){
                        case HANG_CODE:
                            hanging_c(code, server_com);
                            break;

                        case SHOW_CODE:
                            //prelevo username dal buffer
                            sscanf(buffer, "%s %s", command, username);
                            show_c(code, username, server_com);
                            break;

                        case CHAT_CODE:
                            //prelevo username dal buffer
                            sscanf(buffer, "%s %s", command, username);
                            //chat_c(code, server_com, buffer);
                            break;

                        case SHARE_CODE:
                            //prelevo username e filename dal buffer
                            sscanf(buffer, "%s %s %s", command, username, filename);
                            //share_c(code, server_com, buffer);
                            break;

                        case OUT_CODE:
                            //out_c(code, server_com);
                            break;

                        default:
                            printf("+++Comando [%s] non riconosciuto+++\n", command);
                            break;
                    }

                    

                
                    
                }
            }
        }
    }



}