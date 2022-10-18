#include "utils.c"

int DEVICE_PORT;

int main(int argc, char* argv[]){

    char buffer[BUFSIZE];
    char command[MAXCODE];
    int code;
    int caso;

    int server_com, cl_listener;
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
    
   

    return 0;
}