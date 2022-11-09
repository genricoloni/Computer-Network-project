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

/********************************************************************************************************/
/*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<FUNZIONI DI UTILITY PER CLIENT E SERVER>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
/********************************************************************************************************/

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
    else if (strcmp(code, "hanging") == 0)
        return HANG_CODE;
    else if (strcmp(code, "show"))
        return SHOW_CODE;
    else if (strcmp(code, "chat") == 0)
        return CHAT_CODE;
    else if (strcmp(code, "share") == 0)
        return SHARE_CODE;
    else if (strcmp(code, "out") == 0)
        return OUT_CODE;
    else return -1;
    
}


/********************************************************************************************************/
/*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<FUNZIONI DI UTILITY SULLE LISTE>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
/********************************************************************************************************/

//la funzione viene chiamata dal server quando un utente effettua un login: 
//verrà aggiunto alla liusta di utenti online
void inserisci_utente(struct utenti_online **head, char *Username, uint32_t socket, int port)
{
      struct utenti_online *node = (struct utenti_online *)malloc(sizeof(struct utenti_online));
      node->pointer = NULL;
      node->socket = socket;
      node->port = port;
      strcpy(node->username, Username);

      // il nodo puntato da elem è gia inzializzato quando chiamo la routine
      if (*head == NULL)
            *head = node;
      else
      { // piazza elem in testa alla lista
            node->pointer = *head;
            *head = node;
      }
}

//rimuove dalla lista utenti_online un utente che si è disconnesso
void rimuovi_utente(struct utenti_online **head, uint32_t todelete){
      struct utenti_online *pun, *temp;

      if (*head == NULL)
            return;

      // todelete si trova in testa
      if ((*head)->socket == todelete)
      {
            pun = *head;
            *head = (*head)->pointer;
            free(pun);

            return;
      }

     // elemento in mezzo alla lista

      
      for (pun = *head; pun != NULL; pun = pun->pointer, temp = pun)
            if (pun->socket == todelete)
                  break;

      // non ho trovato nulla
      if (pun == NULL)
            return;

      // l'elemento è stato trovato
      // elimino
      temp->pointer = pun->pointer;
      return;
}

// aggiorna i valori della history quando il client fa login
void aggiorna_registro_utente(char *Username, int port){
      struct user_record record;
      time_t rawtime;
      FILE *fileptr = fopen("registro.txt", "rb+");
      while (fread(&record, sizeof(struct user_record), 1, fileptr))
      {
            if (strcmp(record.Username, Username) == 0)
            {
                  // ho trovato il record che cercavo e quindi ne aggiorno il campo timestamp_in
                  record.timestamp_in = c_time(&rawtime);
                  record.timestamp_out = 0;
                  record.Port = port;
                  printf(" Sto scrivendo sul registro client history i seguenti valori\nUsername:%s\nTimestampIN:%ld\nTimestampOUT:%ld\nPort:%d\n ",
                         record.Username,
                         record.timestamp_in,
                         record.timestamp_out,
                         record.Port);
                  fseek(fileptr, -1 * sizeof(struct user_record), SEEK_CUR);
                  fwrite(&record, sizeof(struct user_record), 1, fileptr);
                  fclose(fileptr);
                  return ;
            }
}
      fclose(fileptr);
      return;
}





/********************************************************************************************************/
/*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<FUNZIONI DI UTILITY PER SERVER>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
/********************************************************************************************************/
//la funzione restituisce false se l'username non è 
//gia presente tra gli utenti registrati
bool check_presenza_utente(struct credenziali cred){
    FILE *cr = fopen("reg_users.txt", "rb");
    struct credenziali temp_cr;

    while(fread(&temp_cr, sizeof(temp_cr), 1, cr)){
        if(strcmp(cred.username, temp_cr.username) == 0){
            fclose(cr);
            return false;
        }
    }
    fclose(cr);
    return true;
}

//la funzione restituisce true se l'username è gia presente
//tra gli utenti registrati && la password associata è corretta
bool check_login_utente(struct credenziali cred){
    FILE *cr = fopen("reg_users.txt", "rb");
    struct credenziali temp_cr;

    while(fread(&temp_cr, sizeof(temp_cr), 1, cr)){
        if(strcmp(cred.username, temp_cr.username) == 0
        && strcmp(cred.password, temp_cr.password) == 0){
            fclose(cr);
            return true;
        }
    }
    fclose(cr);
    return false;
}

// inizializza un utente nel file history
void inizializza_history(struct credenziali credenziali)
{


      FILE *fptr = fopen("registro.txt", "ab"); // modalità append per non sovrascrivere i record precedenti
      struct user_record record;
      strcpy(record.Username, credenziali.username);
      record.Port = 0;
      record.timestamp_in = 0;
      record.timestamp_out = 0;
      fwrite(&record, sizeof(struct credenziali), 1, fptr);

      fclose(fptr);
}

//la funzione aggiugne l'utente user alla lista degli utenti registrati, con la sua password
//da usare dopo che si è verificato che l'username non è già stato registrato
void registra_utente(struct credenziali cred){
    FILE *fptr = fopen("reg_users.txt", "ab");
      fwrite(&cred, sizeof(cred), 1, fptr);
      fclose(fptr);
}

//la funzione converte il comando server in un codice intero
int codifica_comando_server(char *comando){
    if(strcmp(comando, "list") == 0)
        return LIST_CODE;
    if(strcmp(comando, "help") == 0)
        return HELP_CODE;
    if(strcmp(comando, "esc") == 0)
        return ESC_CODE;
    return -1;
}

//stampa i comandi disponibili per il server
void stampa_comandi_server(){
    printf("<<<<<<<<<<<<<<<<<<COMANDI SERVER>>>>>>>>>>>>>>>>>>\n\n");
    printf("help - mostra una breve descrizione dei comandi\n");
    printf("list - mostra lista utenti online\n");
    printf("esc  - chiude il server\n");  
}

//stampa una breve descrizione dei comandi del server
void stampa_help_server(){
    system("clear");
    printf("comando 'LIST':\n mostra la lista degli utenti online, specificando il loro username, la porta su cui sono connessi e il timestamp di connessione\n");

    printf("comando 'ESC':\n chiude il server. Le chat tra gli utenti continueranno in P2P, ma gli utenti non potranno avviare nuove chat\n");

}

/********************************************************************************************************/
/*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<FUNZIONI DELLE OPERAZIONI DEL SERVER>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
/********************************************************************************************************/

//effetta l'operazione di registrazione delle credenziali di un utente che intende registrarsi al servizio
void signup_s(int sock){
    int res;
    uint32_t msg_len, res_t;
    struct credenziali cred;

    res = ACK;
    res_t = htonl(res);
    
    send(sock, (void*)&res_t, sizeof(uint32_t), 0);
    
    msg_len = (sizeof(cred));
    //ricevo le credenziali
    recv(sock, (void*)&cred, msg_len, 0);

    fflush(stdout);
    
    
    if(check_presenza_utente(cred) == false){
        //invio il segnale di errore "utente già registrato"
        res = ALRDY_REG;
        res_t = htonl(res);
        fflush(stdin);
        send(sock, (void*)&res_t, sizeof(uint32_t), 0);
        return;
    }
    fflush(stdin);
    //scrivo le credenziali su file
    registra_utente(cred);
    //invio ACK di conferma
    res = ACK;
    res_t = htonl(res);
    send(sock, (void*)&res_t, sizeof(uint32_t), 0);
    fflush(stdout);
    printf("UTENTE %s REGISTRATO AL SERVIZIO!\n", cred.username);
    return;
    
    }

//effettua l'operazione di log-in di un utente su richiesta di un device
bool login_s(int sock, struct utenti_online **testa){
    int res, port;
    uint32_t msg_len, res_t, port_t     ;
    struct credenziali cred;

    res = ACK;
    res_t = htonl(res);
    //invio ack di ricezione codice
    send(sock, (void*)&res_t, sizeof(uint32_t), 0);

    //ricevo la porta
    recv(sock, (void*)&port_t, sizeof(uint32_t), 0);
    port = ntohl(port_t);


    msg_len = (sizeof(cred));

    //ricevo le credenziali
    recv(sock, (void*)&cred, msg_len, 0);

    
    //controllo correttezza credenziali
    if(check_login_utente(cred) == false){
        //le credenziali non sono corrette
        res = IN_ERR;
        res_t = htonl(res);
        send(sock, (void*)&res_t, sizeof(uint32_t), 0);
        return false;
    }
    res = ACK;
    res_t = htonl(res);
    send(sock, (void*)&res_t, sizeof(uint32_t), 0);

    inserisci_utente(testa, cred.username, sock, port);
    inizializza_history(cred);
    aggiorna_registro_utente(cred.username, port);
    return true;

   
}

void stampa_lista_utenti_online(struct utenti_online *testa){
    struct utenti_online *tmp = testa;
    printf("<<<<<<<<<<<<<<<<<<UTENTI ONLINE>>>>>>>>>>>>>>>>>>\n\n");
    while(tmp != NULL){
        printf("Username: %s\n", tmp->username);
        printf("Porta: %d\n", tmp->port);
        printf("Timestamp: %ld\n\n", tmp->timestamp_in);
        tmp = tmp->pointer;
    }
}




/********************************************************************************************************/
/*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<FUNZIONI DELLE OPERAZIONI DEL CLIENT>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
/********************************************************************************************************/
//la funzione si occupa di estrarre credenziali dalla stringa,
//inviarle al server e aspettare una conferma di avvenuta registrazione
bool signup_c(int code, struct credenziali credenziali, int sock){
    int ack;
    uint32_t msg_len, code_t;

    //serializzazione
    msg_len = sizeof(uint32_t);

    code_t = htonl(code);

    //invio del codice al server
    send(sock, (void*)&code_t, msg_len, 0);

    //aspetto conferma
    recv(sock, (void*)&code_t, sizeof(uint32_t), 0);
    ack = ntohl(code_t);

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

//la funzione si occupa di estrarre le credenziali dalla stringa,
//inviarle al server e aspettare uan conferma di avvenuto log-in
bool login_c(int code, struct credenziali credenziali, int sock, int port){
    int ack;
    uint32_t msg_len, code_t, port_t;

    //serializzazione
    msg_len = sizeof(uint32_t);

    code_t = htonl(code);

    //invio del codice al server
    send(sock, (void*)&code_t, msg_len, 0);

    //aspetto conferma
    recv(sock, (void*)&code_t, sizeof(uint32_t), 0);
    ack = ntohl(code_t);

    if(ack != ACK){
        perror("Errore in fase di comunicazione, riavvio dell'applicazione necessario\n");
        exit(-1);
    }

    //invio la porta
    port_t = htonl(port);

    send(sock, (void*)&port_t, sizeof(uint32_t), 0);

    //serializzazione
    msg_len = htonl(sizeof(credenziali));
    //invio la struct contenente le credenziali
    send(sock, (void*)&credenziali, msg_len, 0);

    //aspetto conferma
    recv(sock, (void*)&code_t, sizeof(uint32_t), 0);
    ack = ntohl(code_t);

    if(ack == IN_ERR)
        return false;
    return true;
}


//funzione che richiede al server tutti i messaggi riferiti al client
//la funzione si occupa di stampare l'username, il numero di messaggi pendenti
//e il timestamp del messassggio più recente
void hanging_c(int code, int sock){
    int ack, hang, count_hang;
    uint32_t msg_len, code_t, hang_t, count_hang_t;
    struct messaggio_pendente mp;
    
    system("clear");
    //serializzazione
    msg_len = sizeof(uint32_t);

    code_t = htonl(code);

    //invio del codice al server
    send(sock, (void*)&code_t, msg_len, 0);

    //aspetto conferma
    recv(sock, (void*)&code_t, sizeof(uint32_t), 0);
    ack = ntohl(code_t);

    
    if(ack != ACK){
        perror("Errore in fase di comunicazione, riavvio dell'applicazione necessario\n");
        exit(-1);
    }

    //ricevo il numero di utenti per i quali ho messaggi pendenti

    recv(sock, (void*)&hang_t, sizeof(uint32_t), 0);
    hang = ntohl(hang_t);

    for(int i = 0; i < hang; i++){
        //ricevo il nome dell'utente
        recv(sock, (void*)&mp.utente, sizeof(&mp.utente), 0);

        //ricevo il numero di messaggi pendenti
        recv(sock, (void*)&count_hang_t, sizeof(uint32_t), 0);
        mp.messaggi_pendenti = ntohl(count_hang_t);

        //ricevo il timestamp del messaggio
        recv(sock, (void*)&mp.timestamp, sizeof(time_t), 0);

        printf(" %s ha inviato %d messaggi, il più recente è del %s\n\n", mp.utente, count_hang, ctime(&mp.timestamp));

    }
    printf("Premi un tasto per tornare al menù principale\n");
    getchar();

}

//funzione che richiede al server tutti i messaggi riferiti al client da un utente specifico
//i messaggi vengono memorizzati nel file di chat tra i due utenti
void show_c(int code, char *utente, int sock){
    int ack, count_msg;
    uint32_t msg_len, code_t, count_msg_t;
    FILE *fp;
    char path[100];


    msg_len = sizeof(uint32_t);

    code_t = htonl(code);

    //invio del codice al server
    send(sock, (void*)&code_t, msg_len, 0);

    //aspetto conferma
    recv(sock, (void*)&code_t, sizeof(uint32_t), 0);
    ack = ntohl(code_t);

    
    if(ack != ACK){
        perror("Errore in fase di comunicazione, riavvio dell'applicazione necessario\n");
        exit(-1);
    }

    //invio il nome dell'utente
    send(sock, (void*)&utente, sizeof(&utente), 0);

    //ricevo il numero di messaggi
    recv(sock, (void*)&count_msg_t, sizeof(uint32_t), 0);
    count_msg = ntohl(count_msg_t);

    //apro il file di chat
    //sprintf(path, "%s_%s", cred.username, utente);
    fp = fopen(path, "a");

    //for(int i = 0; i < count_msg; i++)
    {
        //ricevo il messaggio


        //scrivo il messaggio nel file di chat

    }

    fclose(fp);

}



