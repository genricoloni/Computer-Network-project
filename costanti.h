#ifndef COSTANTI_H
#define COSTANTI_H 
#define BUFSIZE         1024
#define MAXCMD          9   //lunghezza massima della stringa che può contenere un comando
#define MAXPORT         7   //può valere al massimo 60000

#define USERN_CHAR   16  //massimo numero di caratteri per l'username
#define PW_CHAR         8   //massimo numero di caratteri per la password

#define ALRDY_REG       -1
#define IN_ERR          -1
#define ACK             77  //codice di conferma della correttezza delle operazioni
#define ERR_CODE        -1  //codice assegnato quando non viene riconosciuto un comando
#define SIGNUP_CODE     1
#define IN_CODE         2
#define HANG_CODE       3
#define SHOW_CODE       4   
#define CHAT_CODE       5
#define SHARE_CODE      6
#define OUT_CODE        7
#define MSG_CODE        90
#define ADD_CODE        91

#define HELP_CODE       11
#define LIST_CODE       12
#define ESC_CODE        7

#define STDIN 0

struct utenti_online{
    char username[USERN_CHAR];
    int socket;
    int port;
    time_t timestamp_in;
    struct utenti_online* pointer;
};

// le credenziali che devono essere inviate vengono memorizzate dal client in questa struttura dati
struct credenziali{
    char username[USERN_CHAR];
    char password[PW_CHAR];
};



//struttura analoga a credenziali, utile per l'invio tramite socket
struct credenziali_t{
    u_int8_t user[USERN_CHAR];
    u_int8_t pass[PW_CHAR];
};

// descrittore record di history di un utente (viene utilizzato per tenere traccia dei login e dei logout)
struct user_record{
      char Username[USERN_CHAR];
      int Port;
      char timestamp_in[25];
      char timestamp_out[25];
};

//la struttura viene aggiornata ogni qualvolta viene inviato ad un utente attualmente offline
struct messaggio_pendente{
    char utente[USERN_CHAR];
    int messaggi_pendenti; //il numero dei messaggi pendenti, viene incrementato ogni qualvolta viene inviato un messaggio o azzerato quando viene chiamata la show dal client
    time_t timestamp;
};

//ogni messaggio inviato (sia ai client che al server) è composto dall'username
//del mittente, dal timestamp e dal messaggio vero e proprio
struct sent_message{
    char utente[USERN_CHAR];
    char messaggio[BUFSIZE];
    time_t timestamp;
};

//lista contenente tutti i destinatari di un messaggio
//la chat singola è un caso particolare di chat di gruppo
struct destinatari{
    char username[USERN_CHAR];
    int socket;
    struct destinatari* next;
};


#endif /* CHAT_H */
