#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "costanti.h"

typedef enum { false, true } bool;
int OWN_PORT;
//la lista dei destinatari deve essere visibile globalmente
struct destinatari* destinatari = NULL;

struct utenti_online* utenti_online = NULL;



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
    else if (strcmp(code, "show") == 0)
        return SHOW_CODE;
    else if (strcmp(code, "chat") == 0)
        return CHAT_CODE;
    else if (strcmp(code, "share") == 0)
        return SHARE_CODE;
    else if (strcmp(code, "out") == 0)
        return OUT_CODE;
    else return -1;
    
}

//funzione che ferma il programma in attesa della pressione di un tasto
void wait(){
    fflush(stdin);
    printf("Premi INVIO per tornare al menù principale\n");
    getchar();
}


/********************************************************************************************************/
/*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<FUNZIONI DI UTILITY SULLE LISTE>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
/********************************************************************************************************/

//la funzione viene chiamata dal server quando un utente effettua un login: 
//verrà aggiunto alla liusta di utenti online
void inserisci_utente(struct utenti_online **head, char *Username, uint32_t socket, int port)
{
    time_t curtime;
    struct utenti_online *node = (struct utenti_online *)malloc(sizeof(struct utenti_online));
    node->pointer = NULL;
    node->socket = socket;
    node->port = port;
    node->timestamp_in = time(&curtime);
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

//la funzione viene chiamata dal client: aggiunge username alla lista dei destinatari
void inserisci_destinatario( char *Username, int socket)
{
    struct destinatari *node = (struct destinatari *)malloc(sizeof(struct destinatari));
    node->socket = socket;
    node->next = NULL;
    strcpy(node->username, Username);
    // il nodo puntato da elem è gia inzializzato quando chiamo la routine
    if (destinatari == NULL)
          destinatari = node;
    else
    { // piazza elem in testa alla lista
          node->next = destinatari;
          destinatari = node;
    }
}

/********************************************************************************************************/
/*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<FUNZIONI DI UTILITY PER SERVER>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
/********************************************************************************************************/

// aggiorna i valori della history quando il client fa login
void aggiorna_registro_utente(char *Username, int port){
      struct user_record record;
      time_t rawtime;
      FILE *fileptr = fopen("./sources_s/registro.txt", "rb+");
      while (fread(&record, sizeof(struct user_record), 1, fileptr))
      {
            if (strcmp(record.Username, Username) == 0)
            {
                // ho trovato il record che cercavo e quindi ne aggiorno il campo timestamp_in
                //trovo il timestamp attuale e lo assegno alla variabile
                time(&rawtime);
                record.timestamp_in = ctime(&rawtime);
                record.timestamp_out = 0;
                record.Port = port;
                printf("Debug: timestamp-> %s\n", record.timestamp_in);
                fseek(fileptr, -1 * sizeof(struct user_record), SEEK_CUR);
                fwrite(&record, sizeof(struct user_record), 1, fileptr);
                fclose(fileptr);
                return ;
            }
}
      fclose(fileptr);
      return;
}

//la funzione restituisce false se l'username non è 
//gia presente tra gli utenti registrati
/*bool check_presenza_utente(char* username){
    FILE *cr = fopen("./sources_s/reg_users.txt", "r");
    struct credenziali temp_cr;
    fflush(stdout);
    while(fread(&temp_cr, sizeof(temp_cr), 1, cr)){
        if(strcmp(username, temp_cr.username) == 0){
            fclose(cr);
            return false;
        }
    }
    fclose(cr);
    return true;
}*/

//la funzione controlla che l'username appartenga ad un utente registrato
bool check_presenza_utente(char* username){
    FILE *cr = fopen("./sources_s/reg_users.txt", "r");
    struct credenziali temp_cr;
    fflush(stdout);
    while(fread(&temp_cr, sizeof(temp_cr), 1, cr)){
        if(strcmp(username, temp_cr.username) == 0){
            fclose(cr);
            return true;
        }
    }
    fclose(cr);
    return false;
}

//la funzione restituisce true se l'username è gia presente
//tra gli utenti registrati && la password associata è corretta
bool check_login_utente(struct credenziali cred){
    FILE *cr = fopen("./sources_s/reg_users.txt", "r");
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
    FILE *fptr = fopen("./sources_s/reg_users.txt", "a");
    printf("Debug: sto scrivendo nel file reg_users.txt\n");
    fwrite(&cred, sizeof(cred), 1, fptr);
    printf("Debug: ho scritto nel file reg_users.txt\n");
    fclose(fptr);
    printf("Debug: ho chiuso il file reg_users.txt\n");
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
    wait();
    //system("clear");
}

//la funzione restituisce l'username dato il socket
char *get_username(int socket){
    struct utenti_online *temp = utenti_online;
    while(temp != NULL){
        if(temp->socket == socket)
            return temp->username;
        temp = temp->pointer;
    }
    return NULL;
}

//la funzione restituisce la porta dato l'username
int get_port(char *username){
    struct utenti_online *temp = utenti_online;
    while(temp != NULL){
        if(strcmp(temp->username, username) == 0)
            return temp->port;
        temp = temp->pointer;
    }
    return -1;
}

//la funzione restituisce true se l'utente è online
bool check_online(char *username){
    struct utenti_online *temp = utenti_online;
    while(temp != NULL){
        if(strcmp(temp->username, username) == 0)
            return true;
        temp = temp->pointer;
    }
    return false;
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
    
    
    if(check_presenza_utente(cred.username) == false){
        //invio il segnale di errore "utente già registrato"
        res = ALRDY_REG;
        res_t = htonl(res);
        fflush(stdin);
        send(sock, (void*)&res_t, sizeof(uint32_t), 0);
        return;
    }
    fflush(stdout);
    //scrivo le credenziali su file
    registra_utente(cred);
    printf("ho registrato l'utente %s\n", cred.username);
    //invio ACK di conferma
    res = ACK;
    res_t = htonl(res);
    send(sock, (void*)&res_t, sizeof(uint32_t), 0);
    printf("ho inviato l'ACK\n");
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

    printf("RICEVUTE CREDENZIALI UTENTE %s\n", cred.username);
    //controllo correttezza credenziali
    if(check_login_utente(cred) == false){
        //le credenziali non sono corrette
        res = IN_ERR;
        res_t = htonl(res);
        printf("Debug: non ci sono credenziali\n");
        send(sock, (void*)&res_t, sizeof(uint32_t), 0);
        return false;
    }
    res = ACK;
    res_t = htonl(res);
    send(sock, (void*)&res_t, sizeof(uint32_t), 0);

    inserisci_utente(testa, cred.username, sock, port);
    inizializza_history(cred);
    aggiorna_registro_utente(cred.username, port);
    printf("Debug dopo aggiorna registro utente\n");
    return true;

   
}

void stampa_lista_utenti_online(struct utenti_online *testa){
    struct utenti_online *tmp = testa;
    bool someone = false;
    fflush(stdin);
    system("clear");
    while(tmp != NULL){
        someone = true;
        printf("Username: %s\n", tmp->username);
        printf("Porta: %d\n", tmp->port);
        printf("Timestamp: %s\n\n", ctime(&tmp->timestamp_in));
        tmp = tmp->pointer;

    }

    if(someone == false)
        printf("NESSUN UTENTE ONLINE\n");

    wait();

}

//inizializza la chat tra due utenti:
//se il destinatario è offline il server lo memorizza come messaggio pendente
//altrimenti invia al client le informazioni per avviare la chat P2P
void chat_s(int socket){
    int res, port;
    uint32_t msg_len, res_t, port_t;
    char destinatario[USERN_CHAR];
    char mittente[USERN_CHAR];
    char* path;
    struct credenziali cred;
    
    
    
    res = ACK;
    res_t = htonl(res);
    //invio ack di ricezione codice
    send(socket, (void*)&res_t, sizeof(uint32_t), 0);

    //ricevo username del destinatario
    recv(socket, (void*)&cred, sizeof(cred), 0);
    strcpy(destinatario, cred.username);
    printf("RICEVUTO USERNAME DESTINATARIO: %s\n", destinatario);

    //verifico che il destinatario esista

    if(!check_presenza_utente(destinatario) == true){
        //il destinatario non esiste
        res = IN_ERR;
        res_t = htonl(res);
        send(socket, (void*)&res_t, sizeof(uint32_t), 0);
        return;}
    
    //il destinatario esiste
    res = ACK;
    res_t = htonl(res);
    send(socket, (void*)&res_t, sizeof(uint32_t), 0);
    

    //invio username del mittente
    strcpy(mittente, get_username(socket));
    msg_len = sizeof(&mittente);
    send(socket, (void*)&mittente, msg_len, 0);


    //verifico che il destinatario sia online
    if(check_online(destinatario) == true){
        //il destinatario è online
        res = ACK;
        res_t = htonl(res);
        send(socket, (void*)&res_t, sizeof(uint32_t), 0);
        //invio al client le informazioni per avviare la chat P2P
        port = get_port(destinatario);
        printf("Debug: la porta del destinatario è %d\n", port);
        port_t = htonl(port);
        send(socket, (void*)&port_t, sizeof(uint32_t), 0);
        return;
    } 
    //notifico il client che username non è online
    res = ERR_CODE;
    res_t = htonl(res);
    send(socket, (void*)&res_t, sizeof(uint32_t), 0);
    //memorizzo il messaggio come messaggio pendente
    
    
        
}





//funzione usata dal server quando un client notifica la sua disconnessione
//l'utente viene rimosso dalla lista degli utenti online
//e viene aggiornato il file relativo al registro degli utenti
void out_s(struct utenti_online **testa, char *username){
    struct utenti_online *tmp = *testa;
    struct utenti_online *prec = NULL;
    struct user_record *tmp_r = NULL;
    time_t curtime;
    FILE *fp = fopen("utenti_online.txt", "a");

    strcpy(tmp_r->Username, username);
    


    while(tmp != NULL){
        if(strcmp(tmp->username, username) == 0){
            //recupero timestamp di connessione
            tmp_r->timestamp_in = ctime(&tmp->timestamp_in);
            //recupero la porta sul quale il device era connesso
            tmp_r->Port = tmp->port;
            if(prec == NULL){
                *testa = tmp->pointer;
                free(tmp);
                return;
            }
            prec->pointer = tmp->pointer;
            free(tmp);
            return;
        }
        prec = tmp;
        tmp = tmp->pointer;
    }

    //aggiorno timestamp di disconnessione
    tmp_r->timestamp_out = ctime(&curtime);

    //scrivo su file
    fprintf(fp, "%s %d %s %s", tmp_r->Username, tmp_r->Port, tmp_r->timestamp_in, tmp_r->timestamp_out);

    return;


}


/********************************************************************************************************/
/*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<FUNZIONI DI UTILITY DEL CLIENT>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
/********************************************************************************************************/

//la funzione si occupa di gestire la chat vera e propria:
//deve poter inviare più messaggi, ricevere più messaggi e poter terminare la chat;
//inoltre deve poter ricevere messaggi anche quando non è in attesa di un input da tastiera.
//per questo motivo è stata utilizzata la select:
//la select permette di gestire più socket contemporaneamente
//e di poter ricevere messaggi anche quando non si è in attesa di un input da tastiera
void chat_c(int socket){

}




//funzione che stampa la cronologia della chat da un file dato in input
void print_chat(char *path){
    FILE *fp;
    char c;

    fp = fopen(path, "r");

    while((c = fgetc(fp)) != EOF)
        printf("%c", c);

    fclose(fp);
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

    if(ack != ACK)
        return false;
    return true;
}


//funzione che richiede al server tutti i messaggi riferiti al client
//la funzione si occupa di stampare l'username, il numero di messaggi pendenti
//e il timestamp del messassggio più recente
void hanging_c(int code, int sock){
    int ack, hang, i;
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

    for(i = 0; i < hang; i++){
        //ricevo il nome dell'utente
        recv(sock, (void*)&mp.utente, sizeof(&mp.utente), 0);

        //ricevo il numero di messaggi pendenti
        recv(sock, (void*)&count_hang_t, sizeof(uint32_t), 0);
        mp.messaggi_pendenti = ntohl(count_hang_t);

        //ricevo il timestamp del messaggio
        recv(sock, (void*)&mp.timestamp, sizeof(time_t), 0);

        printf(" %s ha inviato %d messaggi, il più recente è del %s\n\n", mp.utente, mp.messaggi_pendenti, ctime(&mp.timestamp));

    }
    printf("Premi un tasto per tornare al menù principale\n");


}



//funzione che richiede al server tutti i messaggi riferiti al client da un utente specifico
//i messaggi vengono memorizzati nel file di chat tra i due utenti
void show_c(int code, char *utente, int sock){
    int ack, count_msg, i;
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

    for(i = 0; i < count_msg; i++)
    {
        //ricevo il messaggio


        //scrivo il messaggio nel file di chat

    }

    fclose(fp);

}


//funzione che inizializza una chat tra due utenti;
//la funzione si occupa di creare il file di chat tra i due utenti
//e di inviare al server il nome dell'utente con cui si vuole chattare
int chat_init_c(int code, char* username, int server_sock){
    int ack;
    struct credenziali cred;
    uint32_t code_t, msg_len;
    bool exist;
    char path[100], user[USERN_CHAR];
    
    code_t = htonl(code);
    
    //invio codice al server
    send(server_sock, (void*)&code_t, sizeof(uint32_t), 0);

    //aspetto conferma
    recv(server_sock, (void*)&code_t, sizeof(uint32_t), 0);
    ack = ntohl(code_t);

    if(ack != ACK){
        perror("Errore in fase di comunicazione col server\n");
        return -1;
    }
    printf("username e' %s\n", username);
    
    //invio il nome dell'utente con cui si vuole chattare
    strcpy(cred.username, username);
    
    send(server_sock, (void*)&cred, sizeof(&cred), 0);

    //il server mi dice se l'utente esiste
    recv(server_sock, (void*)&code_t, sizeof(uint32_t), 0);
    ack = ntohl(code_t);

    if(ack != ACK){
        printf("L'utente non esiste\n");
        return -1;
    }
    printf("prima della recv\n");
    //ricevo il mio username
    recv(server_sock, (void*)&user, sizeof(&user), 0);

    printf("Dopo recv\n");
    //verifico se il file di chat con username esiste già;
    //se esiste già, stampo la cronologia della chat

    /*path = strcat(path, user);
    path = strcat(path, "/chat/");
    path = strcat(path, username);*/
    //memset(path, 0, sizeof(path));
    printf("dopo memset\n");
    sprintf(path, "./%s/chat/%s", user, username);
    printf("Dopo sprintf path vale %s\n", path);
    if(access(path, F_OK) == 0){
        exist = true;
        //aggiorno, se non è già stato fatto, la cronologia della chat
        //chiamando la show
        //il file esiste già, dunque
        //stampo la cronologia della chat
        print_chat(path);
    }
    printf("exist vale %d\n", exist);

    //parametro da mettere nella funzione di chat, indica se serve o meno creare un file di chat
    


    //il server mi informa se l'utente è online tramite is_on
    recv(server_sock, (void*)&code_t, sizeof(uint32_t), 0);
    ack = ntohl(code_t);


    if(ack != ACK){
        //il client invierà i messaggi al server, che li memorizza in attesa che l'utente si connetta
        printf("L'utente %s non è online, i messaggi saranno inviati al server e verranno consegnati quando %s si connetterà\n", username, username);
        
        inserisci_destinatario("server", server_sock);
        return 0;
        //chiamo una sottofunzione che si occupa della chat vera e propria

    }
    if(ack == ACK){
        //devo connettermi all'utente
        int port, dest_sock;
        uint32_t port_t;

        //il client invierà i messaggi direttamente all'utente
        printf("%s è online, i messaggi gli saranno inviati direttamente\n", username);

        
        //recupero la porta dell'utente 
        recv(server_sock, (void*)&port_t, sizeof(uint32_t), 0);
        port = ntohl(port_t);
       
        printf("porta ricevuta %d\n", port);
        
        //pulizia della memoria e gestione indirizzi del destinatario
        //memset(&cl_addr, 0, sizeof(cl_addr));
        /*cl_addr->sin_family = AF_INET;
		cl_addr->sin_port = htonl(port);
		inet_pton(AF_INET, "127.0.0.1", &cl_addr->sin_addr);
		dest_sock = socket(AF_INET, SOCK_STREAM, 0);
        */

        printf("dest_sock vale %d\n", dest_sock);

        printf("prima del connect\n");
        //connessione al destinatario
        //dest_sock = connect(dest_sock, (struct sockaddr*)&cl_addr, sizeof(cl_addr));
        printf("dopo il connec: la connect vale%d\n", dest_sock);
        //aggiungo l'utente alla lista dei destinatari
        inserisci_destinatario(username, dest_sock);
        printf("dopo inserisci_destinatario\n");
        //chiamo una sottofunzione che si occupa della chat vera e propria
        //chat_c(dest_sock);
        return port;
    }
    


}


//funzione che notifica al server e ai destinatari che il client si sta disconnettendo
void out_c(int code, int server_sock){
    uint32_t code_t;
    int ack;
    struct destinatari* tmp;

    code_t = htonl(code);

    //invio codice al server
    send(server_sock, (void*)&code_t, sizeof(uint32_t), 0);

    //aspetto conferma
    recv(server_sock, (void*)&code_t, sizeof(uint32_t), 0);
    ack = ntohl(code_t);

    if(ack != ACK){
        perror("Errore in fase di comunicazione col server\n");
        return;
    }
    //chiudo la socket con il server
    close(server_sock);

    //chiudo le socket con i destinatari
    while(destinatari != NULL){
        close(destinatari->socket);
        tmp = destinatari;
        destinatari = destinatari->next;
        free(tmp);
    }

    //chiudo il client
    exit(0);
}