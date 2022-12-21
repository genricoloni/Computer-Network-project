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

int mkdir(const char *pathname, mode_t mode);



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




//funzione che legge una riga da un file e la salva in un buffer
void read_line(FILE *fileptr, char *buffer){
    int i = 0;
    char c;

    while((c = fgetc(fileptr)) != EOF && c != '\n'){
        buffer[i] = c;
        i++;
    }
    buffer[i] = '\0';
}


/********************************************************************************************************/
/*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<FUNZIONI DI UTILITY SULLE LISTE>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
/********************************************************************************************************/

//la funzione viene chiamata dal server quando un utente effettua un login: 
//verrà aggiunto alla lista di utenti online
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

//la funzione viene chiamata dal client: rimuove username dalla lista dei destinatari
void rimuovi_destinatario(char *Username){
      struct destinatari *pun, *temp;

      if (destinatari == NULL)
            return;

      // todelete si trova in testa
      if (strcmp(destinatari->username, Username) == 0)
      {
            pun = destinatari;
            destinatari = destinatari->next;
            free(pun);

            return;
      }

     // elemento in mezzo alla lista

      
      for (pun = destinatari; pun != NULL; pun = pun->next, temp = pun)
            if (strcmp(pun->username, Username) == 0)
                  break;

      // non ho trovato nulla
      if (pun == NULL)
            return;

      // l'elemento è stato trovato
      // elimino
      temp->next = pun->next;
      return;
}

//funzione che rimuove tutti gli elementi dalla lista dei destinatari e chiude i relativi socket
void rimuovi_tutti_destinatari(){
    struct destinatari *temp;
    while(destinatari != NULL){
        close(destinatari->socket);
        temp = destinatari;
        destinatari = destinatari->next;
        free(temp);
    }
}



/********************************************************************************************************/
/*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<FUNZIONI DI UTILITY PER SERVER>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
/********************************************************************************************************/


//la funzione controlla che l'username appartenga ad un utente registrato
bool check_presenza_utente(char* username){
    FILE *cr = fopen("./sources_s/reg_users.txt", "r");
    char buffer[USERN_CHAR];
    fflush(stdout);
    while(fgets(buffer, USERN_CHAR, cr)){
        sscanf(buffer, "%s", buffer);
        if(strcmp(username, buffer) == 0){
            fclose(cr);
            return true;
        }
    }
    fclose(cr);
    return false;
}

//funzione che restituisce il numero di caratteri che compongono la stringa
int string_length(char *string){
    int i = 0;
    while(string[i] != '\0')
        i++;
    return i;
}

//la funzione restituisce true se l'username è gia presente
//tra gli utenti registrati && la password associata è corretta
bool check_login_utente(struct credenziali cred){
    FILE *cr = fopen("./sources_s/reg_users.txt", "r");
    char temp_user[USERN_CHAR], temp_pw[PW_CHAR], buffer[20];
    fflush(stdout);

    //leggo due parole alla volta con fgets
    while(fgets(buffer, 20, cr)){
        sscanf(buffer, "%s %s", temp_user, temp_pw);
        if(strcmp(cred.username, temp_user) == 0){
            if(strcmp(cred.password, temp_pw) == 0){
                fclose(cr);
                return true;
            }


        }
    }

    //leggo all'inizio di una nuova riga
    

    fclose(cr);
    return false;
}


//aggiungo una nuova entry al registro degli utenti 
void inizializza_history(char *Username, int port){
    struct user_record record;
    time_t rawtime, timestamp;
    FILE *fileptr = fopen("./sources_s/registro.txt", "a");

    //aggiungo un record al registro
    timestamp = time(&rawtime);
    strcpy(record.Username, Username);
    //record.timestamp_in = ctime(&timestamp);
    //record.timestamp_out = ctime(&tmp);
    strcpy(record.timestamp_in, ctime(&timestamp));
    //rimuovo \n dalla stringa
    record.timestamp_in[string_length(record.timestamp_in) - 1] = '\0';
    //strcat(record.timestamp_in, "\0");
    printf("Debug: %s in login", record.timestamp_in);

    strcpy(record.timestamp_out, "-");
    printf("Debug: %s", record.timestamp_out);

    
    record.Port = port;
    //faccio la fseek
    fprintf(fileptr, "%s %s %s %d\n", record.Username, record.timestamp_in, record.timestamp_out, record.Port);
    fclose(fileptr);
    return;
}




//la funzione aggiugne tramite fprintf l'utente user alla lista degli utenti registrati, con la sua password
//da usare dopo che si è verificato che l'username non è già stato registrato
void registra_utente(struct credenziali user){
    FILE *cr = fopen("./sources_s/reg_users.txt", "a");
    fprintf(cr, "%s %s", user.username, user.password);
    fprintf(cr, "\n");
    fclose(cr);
    return;
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

    
    if(check_presenza_utente(cred.username) == true){
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

    printf("RICEVUTE CREDENZIALI UTENTE %s\n", cred.username);
    //controllo correttezza credenziali
    if(check_login_utente(cred) == false){
        //le credenziali non sono corrette
        res = IN_ERR;
        res_t = htonl(res);
        send(sock, (void*)&res_t, sizeof(uint32_t), 0);
        return false;
    }
    //le credenziali sono corrette
    res = ACK;
    res_t = htonl(res);
    send(sock, (void*)&res_t, sizeof(uint32_t), 0);

    inserisci_utente(testa, cred.username, sock, port);
    inizializza_history(cred.username, port);
    //aggiorna_registro_utente(cred.username, port);
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

//funzione che gestisce la ricezione dei messaggi pendenti nel server
void append_msg_s(char *mittente, char* destinatario, char *msg){
    FILE *fp, *fp1, *tmp;
    char folder[100], path[100], timestamp[100], line[100], tmp_mitt[USERN_CHAR];
    int count;
    bool found = false;
    time_t rawtime = time(NULL);

    //controlla che esista la cartella del destinatario
    //altrimenti la crea
    strcpy(folder, "./sources_s");
    if(access(folder, F_OK) == 0){
    }
    else{
        mkdir(folder, 0777);
        //creo il file di chat
    }
    strcat(folder, "/");
    strcat(folder, destinatario);
    if(access(folder, F_OK) == 0){
    }
    else{
        mkdir(folder, 0777);
        //creo il file di chat
    }
    strcat(folder, "/chat");
    if(access(folder, F_OK) == 0){
    }
    else{
        mkdir(folder, 0777);
    }
    strcat(folder, "/");
    strcpy(path, folder);
    strcat(path, mittente);
    strcat(path, ".txt");
    //controllo se il file esiste
    if(access(path, F_OK) == 0){
        fp = fopen(path, "a");
    }
    else{
        fp = fopen(path, "w");
    }

    //scrivo il messaggio nel file
    fprintf(fp, "%s\n", msg);
    fclose(fp);
    
    //gestisco il file riepilogativo dei messaggi pendenti relativi al destinatario

    strcat(folder, "pendenti.txt");
    if(access(folder, F_OK) != 0){
        //il file non esiste
        fp1 = fopen(folder, "w");
        strcpy(timestamp, ctime(&rawtime));
        count = 1;
        fprintf(fp1, "%s %d %s", mittente, count, timestamp);
        fclose(fp1);
        return;
    }
    //il file esiste
    fp1 = fopen(folder, "r");
    tmp = fopen("tmp.txt", "w");
    while(fgets(line, sizeof(line), fp1) != NULL){
        sscanf(line, "%s %d %s", tmp_mitt, &count, timestamp);
        if(strcmp(tmp_mitt, mittente) == 0){
            if (found == true){
                continue;
            }
            found = true;
            count++;
            strcpy(timestamp, ctime(&rawtime));
        }
        
        fprintf(tmp, "%s %d %s", mittente, count, timestamp);
        }
    if(found == false){
        strcpy(timestamp, ctime(&rawtime));
        count = 1;
        fprintf(tmp, "%s %d %s", mittente, count, timestamp);
    }
    fclose(fp1);
    fclose(tmp);
    if(rename("tmp.txt", folder) != 0){
        perror("rename");
    }


}





//funzione che si occupa di gestire i messaggi pendenti in arrivo per un utente
void msg_s(int socket){
    char destinatario[USERN_CHAR];
    char mittente[USERN_CHAR];
    char buffer[BUFSIZE];
    char path[100];

    //destinatario del messaggio
    recv(socket, (void*)&destinatario, sizeof(destinatario), 0);

    strcpy(mittente, get_username(socket));

    strcat(destinatario, "\0");
    strcat(mittente, "\0");
    //sprintf(path, "./server/%s/chat/%s.txt", *destinatario, *mittente);
    strcat(path, "./server/");
    strcat(path, destinatario);
    strcat(path, "/chat/");
    strcat(path, mittente);
    strcat(path, ".txt");

    //ricevo il messaggio
    recv(socket, (void*)&buffer, sizeof(buffer), 0);


    append_msg_s(mittente, destinatario, buffer);


}





//la funzione trova la entry relativa ad username nel file registro.txt, aggiorna
//il timestamp di disconnessione e scrive su file
void out_s(char *username){
    time_t rawtime;
    int i = 0;
    char buffer[110],tmp[20],tmp1[20], tmp2[20], tmp3[20], tmp4[20], tmp5[20], tmp6[25], tmp7[20], c;
    FILE *fileptr = fopen("./sources_s/registro.txt", "r"), *tmpf = fopen("./sources_s/tmp.txt", "w");



    memset(buffer, 0, sizeof(buffer));

    while (!feof(fileptr)) {
      // leggi una riga del file
      while((c = fgetc(fileptr)) != '\n' ){
        buffer[i] = c;
        i++;
      }
      
      buffer[i] = '\0';
      // resetta l'indice del buffer e avanza il cursore alla riga successiva del file
      i = 0;
        // leggi i campi della riga
        //è formata da username, timestamp di connessione, timestamp di disconnessione, porta
            sscanf(buffer, "%s %s %s %s %s %s %s %s ", tmp, tmp1, tmp2, tmp3, tmp4 , tmp5, tmp6, tmp7);
        if (strcmp(tmp, username) == 0 && strcmp(tmp6, "-") == 0){
            //verifico che il campo timestamp di disconnessione sia -
            //se è - allora lo aggiorno
            //altrimenti non faccio nulla
            //se tmp6 è il carattere di fine stringa 

                //aggiorno il timestamp di disconnessione
                printf("trovatp\n");
                rawtime = time(NULL);
                strcpy(tmp6, ctime(&rawtime));
                tmp6[strlen(tmp6)-1] = '\0';
                //scrivo su file
                memset(buffer, 0, sizeof(buffer));
                strcat(buffer, tmp);
                strcat(buffer, " ");
                strcat(buffer, tmp1);
                strcat(buffer, " ");
                strcat(buffer, tmp2);
                strcat(buffer, " ");
                strcat(buffer, tmp3);
                strcat(buffer, " ");
                strcat(buffer, tmp4);
                strcat(buffer, " ");
                strcat(buffer, tmp5);
                strcat(buffer, " ");
                strcat(buffer, tmp6);
                strcat(buffer, " ");
                strcat(buffer, tmp7);
                strcat(buffer, "\0");
                printf("prima di fprintf modificante\n");
                fprintf(tmpf, "%s\n", buffer);
                printf("dopo fprintf modificante\n");
                //continue;
        }else{
            //scrivo su file
            printf("prima di fprintf non modificante\n");
            fprintf(tmpf, "%s\n", buffer);
            printf("dopo fprintf non modificante\n");
        }
        //scrivo su file
        

        //leggo il carattere successivo: se è EOF esco dal ciclo
              c = fgetc(fileptr);
        if(c == EOF){
            break;
        }
        //altrimenti porto indietro il cursore di un carattere
        else 
            fseek(fileptr, -1, SEEK_CUR);



    }
        fclose(fileptr);
        fclose(tmpf);
        rename("./sources_s/tmp.txt", "./sources_s/registro.txt");
        return;
}






/********************************************************************************************************/
/*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<FUNZIONI DI UTILITY DEL CLIENT>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
/********************************************************************************************************/

//funzione che scrive nel file di chat il messaggio ricevuto
void append_msg_rcv(char* mittente, char *msg, char*OWN_USER){
    char path[100], buffer[BUFSIZE + 4];

    FILE *fileptr;

    strcpy(path, "./");
    strcat(path, OWN_USER);
    strcat(path, "/chat/");
    strcat(path, mittente);
    strcat(path, ".txt");

    strcpy(buffer, "R: ");
    strncat(buffer, msg, strlen(msg)-1);
    //strcat(buffer, "\0");



    fileptr = fopen(path, "a");
    fprintf(fileptr, "%s\n", buffer);
    fclose(fileptr);
}




//funzione che stampa la cronologia della chat da un file dato in input
void print_chat(char * OWN_USER, char *destinatario){
    FILE *fp;
    char c;

    char folder[100];

    strcpy(folder, "./");
    strcat(folder, OWN_USER);
    if(access(folder, F_OK) == 0){
    }
    else{
        mkdir(folder, 0777);
        //creo il file di chat
    }
    strcat(folder, "/chat/");

    //controllo se la cartella esiste
    if(access(folder, F_OK) == 0){
    }
    else{
        mkdir(folder, 0777);
        //creo il file di chat
    }

    strcat(folder, destinatario);
    strcat(folder, ".txt");

    //controllo se il file esiste
    if(access(folder, F_OK) != -1){
        fp = fopen(folder, "r");
    }
    else{
        fp = fopen(folder, "w");
    }
    fp = fopen(folder, "r");

    while((c = fgetc(fp)) != EOF)
        printf("%c", c);

    fclose(fp);
}

//funzione che aggiunge l'ultimo messaggio ad un file di chat
//se il parametro bool è false, il file viene creato
void append_msg_c(char *msg, char* destinatario, char* OWN_USER){
    FILE *fp;
    char folder[100], path[100];

    //controlla che esista la cartella del destinatario
    //altrimenti la crea
    strcpy(folder, "./");
    strcat(folder, OWN_USER);

    //controllo se la cartella esiste
    if(access(folder, F_OK) == 0){
    }
    else{
        mkdir(folder, 0777);
        //creo il file di chat
    }
    strcat(folder, "/chat/");

    //controllo se la cartella esiste
    if(access(folder, F_OK) == 0){
    }
    else{
        mkdir(folder, 0777);
        //creo il file di chat
    }
    strcpy(path, folder);
    strcat(path, destinatario);
    strcat(path, ".txt");

    //controllo se il file esiste
    if(access(path, F_OK) != -1){
        fp = fopen(path, "a");
    }
    else{
        fp = fopen(path, "w");
    }
    //aggiungo il messaggio alla fine del file
    fprintf(fp, "%s", msg);

    fclose(fp);
    
    
}


//funzione che stampa la lista di comandi del client
void print_menu(char* OWN_USER){
    printf("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<Menu principale>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
    printf("Connesso come %s\n", OWN_USER);
    printf("1)hanging                   -->ricevi i messaggi pendenti quando eri offline\n");
    printf("2)show  username            -->mostra i messaggi inviati da username\n");
    printf("3)chat  username            -->avvia una chat con username\n");
    printf("4)share username file_name  -->condividi il file file_name con tutti gli utenti connessi\n");
    printf("5)out                       -->disconnetti l'utente\n");

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
    printf("Debug: ack = %d\n", ack);
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
    uint32_t code_t;
    char user[USERN_CHAR];
    
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
    
    //invio il nome dell'utente con cui si vuole chattare
    strcpy(cred.username, username);
    
    send(server_sock, (void*)&cred, sizeof(&cred), 0);

    //il server mi dice se l'utente esiste
    recv(server_sock, (void*)&code_t, sizeof(uint32_t), 0);
    ack = ntohl(code_t);

    if(ack != ACK)
        return -1;
    
    //ricevo il mio username
    recv(server_sock, (void*)&user, sizeof(&user), 0);


    //il server mi informa se l'utente è online tramite is_on
    recv(server_sock, (void*)&code_t, sizeof(uint32_t), 0);
    ack = ntohl(code_t);


    if(ack != ACK){
        //il client invierà i messaggi al server, che li memorizza in attesa che l'utente si connetta        
        inserisci_destinatario(username, server_sock);
        return 0;
        //chiamo una sottofunzione che si occupa della chat vera e propria

    }
    else{
        //devo connettermi all'utente
        int port;
        uint32_t port_t;

        //il client invierà i messaggi direttamente all'utente
        
        //recupero la porta dell'utente 
        recv(server_sock, (void*)&port_t, sizeof(uint32_t), 0);
        port = ntohl(port_t);

        //aggiungo l'utente alla lista dei destinatari
        return port;
    }
    


}


//funzione che notifica al server e ai destinatari che il client si sta disconnettendo
void out_c(int code, int server_sock){
    struct destinatari* tmp;

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