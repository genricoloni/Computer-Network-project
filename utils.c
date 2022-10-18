#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "costanti.h"

typedef enum { false, true } bool;

//Funzione che permette di trovare la port
int findPort(int argc, char* argv[]){
    int port = 4242;
    if(argc >= 2){
        port = atoi(argv[1]);
    }

    return port;
}

//Converte il comando testuale in un codice per permettere l'uso dei define
//Se il codice non viene riconosciuto, viene assegnato un codice di errore
int cmd_to_code(char* code){
    if(strcmp(code, "signup") == 0)
        return SIGNUP_CODE;
    else if (strcmp(code, "in") == 0)
        return IN_CODE;
    //da completare con tutti gli altri codici
    else return -1;
    
}


void invio_messaggio(){}


//la funzione si occupa di estrarre credenziali e password dalla stringa,
//inviarle al server e aspettare una conferma di avvenuta registrazione
void singup(int code, struct credenziali credenziali, int sock){
    int ret, ack;
    uint32_t msg_len, code_t;
    struct credenziali_t c_t;

    //serializzazione
    code_t = htonl(code);

    //invio del codice al server
    ret = send(socket, (void*)code_t, sizeof(uint32_t), 0);
    //printf("Ok invio codice");

    //aspetto conferma
    ret = recv(socket, (void*)code_t, sizeof(uint32_t), 0);
    printf("Ok ricezione ack");
    ack = ntohl(code_t);
    if(ack != ACK){
        perror("Errore in fase di comunicazione, riavvio dell'applicazione necessario\n");
        exit(-1);
    }

    //serializzazione
    *c_t.user = htonl(credenziali.username);
    *c_t.pass = htonl(credenziali.password);

    //invio la struct contenente le credenziali
    ret = send(socket, (void*)&c_t, sizeof(c_t), 0);

    //aspetto conferma
    ret = recv(socket, (void*)code, sizeof(uint32_t), 0);
    ack = ntohl(ack);
    if(ack != ACK){
        perror("Errore in fase di registrazione, riavvio dell'applicazione necessario\n");
        exit(-1);
    }
    return;
}

int login(char* buffer, int port){
    /*devo inviare i dati e 
    una volta concluso correttamente
    il processo di login, visualizzare
    il menu principale con le nuove opzioni*/

    return 1;
}
