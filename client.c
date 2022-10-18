#include "utils.c"

int DEVICE_PORT;

int main(int argc, char* argv[]){
    system("clear");
    char command[MAXCMD], port[MAXPORT];
    char buffer[BUFSIZE];
    int code;
    int caso;
    int server_com, cl_listener, ret;
    int fdmax = 0;

    struct credenziali credenziali;

    bool connected = false, conn_error = false, cmd_err = false;

    // strutture per indirizzi
    struct sockaddr_in server_addr, client_addr, client_listener_addr, gp_addr;

    fd_set master, readfds;

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
    memset(&client_listener_addr, 0, sizeof(client_listener_addr));
    client_listener_addr.sin_family = AF_INET;
    client_listener_addr.sin_port = DEVICE_PORT;
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
        conn_error = false;
        cmd_err = false;
        fgets(buffer, 1024 - 1, stdin);
		sscanf(buffer, "%s %s %s %s", command, port, credenziali.username, credenziali.password);
		
         
      
        //estraggo la seconda 'parola' dalla stringa inserita, che contiene la porta
        code = cmd_to_code(command);

        if(connected == false){

            //mi connetto al server usando la porta inerita in input
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
                singup(code, user, password, server_com);
                break;

            case IN_CODE:
                //in caso di successo della in posso passare al menu principale e uscire dunque da questo loop
                caso = login(buffer, DEVICE_PORT);
                break;

            default:
                cmd_err = true;
                break;
    }   
        if(caso == 1)
            break;
        



    }
    





    return 0;
}