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


//la funzione restituisce false se l'username non è 
//gia presente nela lista di utenti registrati
bool check_presenza_utente(struct credenziali cred){
    FILE *cr = fopen("reg_users.txt", "rb");
    struct credenziali temp_cr;

    while(fread(&temp_cr, sizeof(temp_cr), 1, cr)){
        if(strcmp(cred.username, temp_cr.username) == 0){
            return false;
        }
    }
    return true;
}


//la funzione aggiugne l'utente user alla lista degli utenti registrati, con la sua password
//da usare dopo che si è verificato che l'username non è già stato registrato
void registra_utente(struct credenziali cred){
    FILE *fptr = fopen("reg_users.txt", "ab");
      fwrite(&cred, sizeof(cred), 1, fptr);
      fclose(fptr);
}



//effetta l'operazione di registrazione delle credenziali di un utente che intende registrarsi al servizio
void signup_s(int sock){
    int res;
    uint32_t msg_len, res_t;
    struct credenziali cred;
    printf("Dentro Signup\n");

    res = ACK;
    res_t = htonl(res);
    
    send(sock, (void*)&res_t, sizeof(uint32_t), 0);
    
<<<<<<< HEAD
    msg_len = (sizeof(cred));
=======
    msg_len = ntohl(sizeof(cred));
>>>>>>> e7c7997e51e8f8ea31c6de959ecb44b6615cfcd0
    //ricevo le credenziali
    recv(sock, (void*)&cred, msg_len, 0);

    printf("Credenziali ricevute:\n %s \n %s\n", cred.username, cred.password);
<<<<<<< HEAD
    fflush(stdout);
=======

>>>>>>> e7c7997e51e8f8ea31c6de959ecb44b6615cfcd0
    
    
    if(check_presenza_utente(cred) == false){
        //invio il segnale di errore "utente già registrato"
<<<<<<< HEAD
        printf("dentro if di check credenziali\n");
        res = ALRDY_REG;
        res_t = htonl(res);
        fflush(stdin);
        printf("prima di send\n");
        send(sock, (void*)&res_t, sizeof(uint32_t), 0);
        printf("dopo send\n");
        return;
    }fflush(stdin);
=======
        res = ALRDY_REG;
        res_t = htonl(res);
        
        send(sock, (void*)&res_t, sizeof(uint32_t), 0);
        return;
    }
>>>>>>> e7c7997e51e8f8ea31c6de959ecb44b6615cfcd0
    printf("dopo check presenza\n");
    //scrivo le credenziali su file
    registra_utente(cred);
    //invio ACK di conferma
    res = ACK;
    res_t = htonl(res);
    printf("prima della send\n");
    send(sock, (void*)&res_t, sizeof(uint32_t), 0);
    printf("dopo send\n");
<<<<<<< HEAD
    fflush(stdout);
=======
    fflush(stdin);
>>>>>>> e7c7997e51e8f8ea31c6de959ecb44b6615cfcd0
    return;
    
    
    //verifico che l'utente non sia già registrato
    
    }




//la funzione si occupa di estrarre credenziali e password dalla stringa,
//inviarle al server e aspettare una conferma di avvenuta registrazione
bool signup_c(int code, struct credenziali credenziali, int sock){
    int ack;
    uint32_t msg_len, code_t;


    //serializzazione

    printf("%s\n", credenziali.username);

    msg_len = sizeof(uint32_t);

    code_t = htonl(code);

    //invio del codice al server
    send(sock, (void*)&code_t, msg_len, 0);
    printf("dopo send\n");
    //printf("Ok invio codice");f

    //aspetto conferma
    recv(sock, (void*)&code_t, sizeof(uint32_t), 0);
<<<<<<< HEAD
    printf("Ok ricezione ack\n");
    ack = ntohl(code_t);
    printf("%d\n", ack);
=======
    printf("Ok ricezione ack");
    ack = ntohl(code_t);
    printf("%d", ack);
>>>>>>> e7c7997e51e8f8ea31c6de959ecb44b6615cfcd0
    if(ack != ACK){
        perror("Errore in fase di comunicazione, riavvio dell'applicazione necessario\n");
        exit(-1);
    }

    //serializzazione
    msg_len = htonl(sizeof(credenziali));
    //invio la struct contenente le credenziali
    send(sock, (void*)&credenziali, msg_len, 0);

    //aspetto conferma
    recv(sock, (void*)&code_t, sizeof(uint32_t), 0);
    ack = ntohl(code_t);
    if(ack == ALRDY_REG )
        return true;
    
    return false;
}

int login(char* buffer, int port){
    /*devo inviare i dati e 
    una volta concluso correttamente
    il processo di login, visualizzare
    il menu principale con le nuove opzioni*/

    return 1;
}
