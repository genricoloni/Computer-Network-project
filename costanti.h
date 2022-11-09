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
      char Username[50];
      uint32_t Port;
      char* timestamp_in;
      char* timestamp_out;
};

struct messaggio_pendente{
    char* utente;
    int messaggi_pendenti;
    time_t timestamp;
};


#endif /* CHAT_H */
