#include <ctype.h>
#include <locale.h>     //Funzione per la data
#include <math.h>       //Funzione abs
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>     //Funzione rand
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>       //Funzioni srand e ctime
#include <unistd.h>

const int DEFAULT = 6;

void control_argv(char argv[]);
int control_maxtry(char argv1[], char argv2[]);
int checkmex(int simpleChildSocket, char mainbuffer[], char* word);
int wordscmp(int simpleChildSocket, int ntry, int maxtry, char mainbuffer[], char* word);
//Altre 2 funzioni: consolewrite e mexcontainer

int main(int argc, char* argv[])
{
    time_t ticks = time(NULL);
    struct tm *tm = localtime(&ticks);
    struct sockaddr_in simpleServer;
    char buffer[260], word[6], data[100];
    char* library[256] = { "abaco", "etere", "mosca", "casco", "ovale", "tappo", "petto", "opera", "danza", "corna", "aroma", "gamba", "razzo", "mango", "bacio", "oliva", "linea", "donna", "croce", "veste", "cesto", "volpe", "udito", "lutto", "cozza", "prete", "rampa", "odore", "treno", "mezzo", "bordo", "sorte", "grata", "sosia", "multa", "crepa", "fiala", "dente", "marea", "tenda", "isola", "cocco", "ambra", "muffa", "panca", "limbo", "mensa", "verme", "cedro", "polso", "volto", "fungo", "vasca", "posta", "zanna", "tonno", "rombo", "ferro", "corvo", "bruco", "latte", "frana", "pasto", "pista", "varco", "campo", "iodio", "asino", "gomma", "sonno", "ladro", "dolce", "esame", "tigre", "menta", "suola", "legno", "ballo", "sfera", "zucca", "caldo", "alghe", "hobby", "rosso", "stufa", "gioco", "fango", "mutuo", "gemma", "pozzo", "cassa", "iride", "frigo", "reato", "trama", "alano", "bosco", "marte", "cloro", "mucca", "icona", "culla", "crema", "succo", "lepre", "hotel", "clima", "palla", "zappa", "salma", "fogna", "dieta", "museo", "gonna", "cella", "villa", "varco", "borsa", "ulivo", "pugno", "cespo", "magia", "felpa", "becco", "torre", "molla", "ruspa", "coppa", "kebab", "ombra", "baule", "zaino", "segno", "cielo", "gallo", "libro", "asilo", "mazzo", "ozono", "legge", "acqua", "ditta", "corte", "penna", "scala", "folla", "larva", "scopa", "mappa", "gatto", "botte", "magma", "falco", "carta", "porta", "jolly", "notte", "torta", "bagno", "pesca", "sarto", "plico", "disco", "monte", "scafo", "tazza", "lampo", "calza", "bimbo", "vetta", "ostia", "fiume", "lista", "carpa", "fondo", "pompa", "birra", "fiore", "reame", "siero", "vetro", "cuoco", "drago", "talpa", "spada", "pizza", "radio", "barra", "cosmo", "festa", "pinna", "cigno", "spago", "marmo", "trota", "freno", "yacht", "bugia", "letto", "selva", "abete", "vespa", "odore", "collo", "barca", "melma", "ratto", "linfa", "podio", "terra", "nervo", "copia", "scudo", "rissa", "cervo", "stiva", "garza", "zolfo", "forno", "canoa", "prato", "sedia", "nuoto", "tetto", "milza", "guida", "mondo", "vento", "fossa", "piede", "sella", "drone", "brodo", "tosse", "rospo", "pacco", "azoto", "viola", "lisca", "piano", "salsa", "corda", "lacca", "zecca", "benda", "falda", "gesso", "masso", "meteo", "lembo", "grano", "ponte", "curva", "pulce", "atomo", "ragno" };
    int simpleSocket = 0, simplePort = 0, returnStatus = 0, maxtry = DEFAULT, ntry, endsignal;

    if (argc < 2 || argc >= 4)       //Controllo numero argomenti
    {
        fprintf(stderr, "\n\nPer avviare il programma sono necessari 2 o 3 argomenti: <nome programma> <porta> <numero tentativo (opzionale)>\n\n<nome programma> equivale a: %s\n<porta> corrisponde ad un valore numerico compreso tra 1024 e 65535\n<numero tentativo>, se viene specificato, ha un valore numerico compreso tra 6 e 10, altrimenti viene assegnato 6 come valore di default\n\nGli argomenti, inoltre, devono essere separati fra di loro con degli spazi.\n\n\n", argv[0]);
        exit(1);
    }
    else
    {
        if(argc == 2)
            control_argv(argv[1]);      //Controllo 2 argomenti
        else
            maxtry = control_maxtry(argv[1], argv[2]);      //Controllo 3 argomenti
    }

    simpleSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (simpleSocket != -1) 
    {
        simplePort = atoi(argv[1]);

        memset(&simpleServer, '\0', sizeof(simpleServer));
        simpleServer.sin_family = AF_INET;
        simpleServer.sin_addr.s_addr = htonl(INADDR_ANY);
        simpleServer.sin_port = htons(simplePort);

        returnStatus = bind(simpleSocket, (struct sockaddr*)&simpleServer, sizeof(simpleServer));

        if (returnStatus == 0)
        {
            returnStatus = listen(simpleSocket, 5);

            if (returnStatus != -1)
            {
                setlocale(LC_TIME, "it_IT.utf8");
                strftime(data, sizeof(data), "%A %d %B %Y, ore %H:%M:%S", tm);
                fprintf(stdout, "\n\nServer inizializzato con successo in data: %s.\nPremere CTRL+C se si desidera terminare il programma.\n", data);

                while (1)
                {
                    struct sockaddr_in clientName = {0};
                    int simpleChildSocket = 0;
                    int clientNameLength = sizeof(clientName);

                    fprintf(stdout, "\n\n\nIn attesa di connessione...\n\n");

                    simpleChildSocket = accept(simpleSocket, (struct sockaddr*)&clientName, &clientNameLength);

                    if (simpleChildSocket != -1)
                    {
                        fprintf(stdout, "\n\nConnessione effetuata.\n\n");

                        //Stabilisce parola casuale

                        memset(&word, '\0', sizeof(word));

                        srand(time(NULL));
                        strcpy(word, library[rand() % 256]);

                        fprintf(stdout, "\nParola: %s.\nTentativi totali: %d.\n", word, maxtry);

                        ntry = 0; 
                        endsignal = 0;

                        while(ntry <= maxtry && endsignal == 0)     //Ciclo per inviare e leggere i messaggi del client
                        {
                            memset(&buffer, '\0', sizeof(buffer));

                            if(ntry == 0)       //Invio messaggio di benvenuto
                            {
                                sprintf(buffer, "OK %d Connessione stabilita. Benvenuto/a!\n", maxtry);
                                fprintf(stdout, "\nMessaggio di benvenuto: %sNumero caratteri: %lu + \\n.\n", buffer, strlen(buffer) - 1);

                                returnStatus = write(simpleChildSocket, buffer, strlen(buffer));
                                
                                if (returnStatus > 0)
                                {
                                    fprintf(stdout, "\nMessaggio inviato.\n");
                                    ntry++;
                                }
                                else
                                {
                                    fprintf(stderr, "\nErrore: invio messaggio fallito, impossibile stabilire una connessione con il client.\n");
                                    endsignal = 1;
                                }
                            }
                            else        //Legge messaggio dal client e invia risposta
                            {
                                fprintf(stdout, "\n\nIn attesa di risposta...\n\n");

                                returnStatus = read(simpleChildSocket, buffer, sizeof(buffer));     //Legge messaggio dal client

                                if (returnStatus > 0)
                                {
                                    if (returnStatus <= 256)        //Controllo limite caratteri nel messaggio ricevuto
                                    {
                                        fprintf(stdout, "\nTentativo: %d.\n", ntry);
                                        fprintf(stdout, "\nMessaggio ricevuto.\n\nContenuto: %sNumero caratteri: ", buffer);

                                        if(buffer[returnStatus - 1] == '\n')
                                            fprintf(stdout, "%d + \\n.\n", returnStatus - 1);
                                        else
                                            fprintf(stdout, "%d.\n", returnStatus);

                                        endsignal = checkmex(simpleChildSocket, buffer, word);        //Analizza il protocollo ricevuto (WORD testo\n o QUIT\n)
                    
                                        if(endsignal == 0)
                                        {
                                            endsignal = wordscmp(simpleChildSocket, ntry, maxtry, buffer, word);        //Confronta la parola del client con quella del server e invia un messaggio di risposta

                                            if(endsignal == 0)
                                                ntry++;
                                        }
                                    }
                                    else
                                    {
                                        memset(&buffer, '\0', sizeof(buffer));

                                        fprintf(stderr, "\nErrore: overflow caratteri. (%d)\n", returnStatus);
                                        sprintf(buffer, "ERR Errore: il messaggio deve avere un numero di caratteri minore o uguale a 256.\n");
                                        fprintf(stdout, "\nMessaggio di errore: %sNumero caratteri: %lu + \\n.\n", buffer, strlen(buffer) - 1);

                                        returnStatus = write(simpleChildSocket, buffer, strlen(buffer));
                                        
                                        if (returnStatus <= 0)
                                            fprintf(stderr, "\nErrore: invio messaggio fallito, impossibile stabilire una connessione con il client.\n");
                                        else
                                            fprintf(stdout, "\nMessaggio inviato.\n");

                                        endsignal = 1;
                                    }
                                }
                                else
                                {
                                    fprintf(stderr, "\nErrore: impossibile stabilire una connessione con il client.\n");
                                    endsignal = 1;
                                }
                            }
                        }

                        close(simpleChildSocket);
                        fprintf(stdout, "\n\nGioco concluso, connessione terminata.\n");
                    }
                    else
                    {
                        fprintf(stderr, "\n\nErrore: impossibile accettare le connesioni\n\n\n");
                        close(simpleSocket);
                        exit(1);
                    }
                }

                close(simpleSocket);
            }
            else
            {
                fprintf(stderr, "\n\nErrore: impossibile ascoltare le richieste di connessione.\n\n\n");
                close(simpleSocket);
                exit(1);
            }
        }
        else
        {
            fprintf(stderr, "\n\nErrore: impossibile eseguire il bind.\n\n\n");
            close(simpleSocket);
            exit(1);
        }
    }
    else
    {
        fprintf(stderr, "\n\nErrore: impossibile creare il socket.\n\n\n");
        exit(1);
    }

    return 0;
}

void control_argv(char argv[])
{
    int returnStatus = 0;
    int long unsigned checkarg = 0 ;

    while (checkarg < strlen(argv) && returnStatus == 0)     //Controllo valori numerici della porta
    {
        if (isdigit(argv[checkarg]) == 0)
        {
            fprintf(stderr, "\n\nErrore: il valore della porta deve essere esclusivamente numerico.\n\n");
            returnStatus = 1;
        }

        checkarg++;
    }

    if (returnStatus == 0)
    {
        if (atoi(argv) < 1024 || atoi(argv) > 65535)        //Controllo range della porta
        {
            fprintf(stderr, "\n\nErrore: il numero della porta deve essere compreso tra i valore 1024 e 65535.\n\n");
            returnStatus = 1;
        }
    }

    if (returnStatus == 1)
    {
        fprintf(stdout, "\n");
        exit(returnStatus);
    }
}

int control_maxtry(char argv1[], char argv2[])
{
    int returnStatus = 0, icsor = 0;
    int long unsigned checkarg = 0;

    while (checkarg < strlen(argv1) && returnStatus == 0)        //Controllo valori numerici della porta
    {
        if (isdigit(argv1[checkarg]) == 0)
        {
            fprintf(stderr, "\n\nErrore: il valore della porta deve essere esclusivamente numerico.\n");
            returnStatus = 1;
            icsor = 1;
        }

        checkarg++;
    }

    if (icsor == 0)
    {
        if (atoi(argv1) < 1024 || atoi(argv1) > 65535)      //Controllo range della porta
        {
            fprintf(stderr, "\n\nErrore: il numero della porta deve essere compreso tra i valore 1024 e 65535.\n");
            returnStatus = 1;
        }
    }

    icsor = 0;
    checkarg = 0;

    while (checkarg < strlen(argv2) && icsor == 0)        //Controllo valori numerici dei tentativi
    {
        if (isdigit(argv2[checkarg]) == 0)
        {
            if(returnStatus == 0)
                fprintf(stdout, "\n");

            fprintf(stderr, "\nErrore: il valore del tentativo deve essere esclusivamente numerico.\n");

            returnStatus = 1;
            icsor = 1;
        }

        checkarg++;
    }

    if (icsor == 0)
    {
        if (atoi(argv2) < 6 || atoi(argv2) > 10)        //Controllo range dei tentativi
        {
            if(returnStatus == 0)
                fprintf(stdout, "\n");

            fprintf(stderr, "\nErrore: se viene specificato il numero dei tentativi, i valori accettabili sono compresi tra 6 e 10.\n");
            returnStatus = 1;
        }
    }

    if (returnStatus == 1)
    {
        fprintf(stdout, "\n\n");
        exit(returnStatus);
    }
    else
        return(atoi(argv2));
}

int checkmex(int simpleChildSocket, char mainbuffer[], char* word)        // Il server riceve i seguenti protocolli: "WORD <testo>\n" o "QUIT\n"
{
    int endsignal = 0, checkchar = 5, returnStatus;
    char buffer[260];
    
    if(mainbuffer[0] == 'W' && mainbuffer[1] == 'O' && mainbuffer[2] == 'R' && mainbuffer[3] == 'D')        //Controllo su WORD
    {
        if(strlen(mainbuffer) == 11)        //Controllo dimensione del messaggio
        {
            if(mainbuffer[4] == ' ')        //Spazio separatore tra protocollo e parola
            {
                if(mainbuffer[10] == '\n')      //Invio finale
                {
                    while(checkchar<10 && endsignal == 0)       //Ciclo per controllare che la stringa del messaggio abbia solo i caratteri alfabetici
                    {
                        if(isalpha(mainbuffer[checkchar]) == 0)
                        {
                            memset(&buffer, '\0', sizeof(buffer));

                            fprintf(stderr, "\nErrore: carattere della parola non consentito.\n");
                            sprintf(buffer, "ERR Errore: nella parola sono ammessi solo caratteri alfabetici.\n");
                            fprintf(stdout, "\nMessaggio di errore: %sNumero caratteri: %lu + \\n.\n", buffer, strlen(buffer) - 1);
                            
                            endsignal = 1;
                        }
                        else
                            checkchar++;
                    }

                    if(endsignal == 0)
                        fprintf(stdout, "\nStruttura messaggio (WORD) corretta.\n");
                }
                else
                {
                    memset(&buffer, '\0', sizeof(buffer));

                    fprintf(stderr, "\nErrore: invio finale (WORD) assente.\n");
                    sprintf(buffer, "ERR Errore: il messaggio non possiede l'invio finale previsto.\n");
                    fprintf(stdout, "\nMessaggio di errore: %sNumero caratteri: %lu + \\n.\n", buffer, strlen(buffer) - 1);
                    
                    endsignal = 1;
                }
            }
            else
            {
                memset(&buffer, '\0', sizeof(buffer));

                fprintf(stderr, "\nErrore: spazio separatore (WORD) assente.\n");
                sprintf(buffer, "ERR Errore: il messaggio non presenta lo spazio divisore tra il protocollo e la parola.\n");
                fprintf(stdout, "\nMessaggio di errore: %sNumero caratteri: %lu + \\n.\n", buffer, strlen(buffer) - 1);
                
                endsignal = 1;
            }
        }
        else
        {
            memset(&buffer, '\0', sizeof(buffer));

            fprintf(stderr, "\nErrore: numero caratteri (WORD+<testo>) non consentito (11 != %lu).\n", strlen(mainbuffer));
            sprintf(buffer, "ERR Errore: il protocollo e/o la parola contengono troppi caratteri.\n");
            fprintf(stdout, "\nMessaggio di errore: %sNumero caratteri: %lu + \\n.\n", buffer, strlen(buffer) - 1);

            endsignal = 1;
        }
    }
    else
    {
        if(mainbuffer[0] == 'Q' && mainbuffer[1] == 'U' && mainbuffer[2] == 'I' && mainbuffer[3] == 'T')        //Controllo su QUIT
        {
            memset(&buffer, '\0', sizeof(buffer));
            
            if(strlen(mainbuffer) == 5)     //Controllo dimensione del messaggio
            {
                if(mainbuffer[4] == '\n')       //Invio finale
                {
                    fprintf(stdout, "\nStruttura messaggio (QUIT) corretta.\n");
                    sprintf(buffer, "QUIT Gioco terminato, la parola era %s.\n", word);
                    fprintf(stdout, "\nResa giocatore.\n\nMessaggio finale: %sNumero caratteri: %lu + \\n.\n", buffer, strlen(buffer) - 1);
                }
                else
                {
                    fprintf(stderr, "\nErrore: invio finale (QUIT) assente.\n");
                    sprintf(buffer, "ERR Errore: il messaggio non possiede l'invio finale previsto.\n");
                    fprintf(stdout, "\nMessaggio di errore: %sNumero caratteri: %lu + \\n.\n", buffer, strlen(buffer) - 1);
                }
            }
            else
            {
                fprintf(stderr, "\nErrore: numero caratteri (QUIT) non consentito (5 != %lu).\n", strlen(mainbuffer));
                sprintf(buffer, "ERR Errore: il protocollo contiene troppi caratteri.\n");
                fprintf(stdout, "\nMessaggio di errore: %sNumero caratteri: %lu + \\n.\n", buffer, strlen(buffer) - 1);
            }
        }
        else
        {
            fprintf(stderr, "\nErrore: protocollo non riconosciuto.\n");
            sprintf(buffer, "ERR Errore: impossibile identificare il protocollo del messaggio.\n");
            fprintf(stdout, "\nMessaggio di errore: %sNumero caratteri: %lu + \\n.\n", buffer, strlen(buffer) - 1);
        }
            
        endsignal = 1;
    }

    if(endsignal == 1)
    {
        returnStatus = write(simpleChildSocket, buffer, strlen(buffer));        //Invia il messaggio al client

        if(returnStatus <= 0)       //Controllo la connessione con il client
            fprintf(stderr, "\nErrore: invio messaggio fallito, impossibile stabilire una connessione con il client.\n");
        else
            fprintf(stdout, "\nMessaggio inviato.\n");
    }

    return endsignal;
}

int wordscmp(int simpleChildSocket, int ntry, int maxtry, char mainbuffer[], char* word)
{
    int endsignal = 0, countchar, posbuffer, posword, returnStatus; 
    char ssign[6], buffer[260];

    memset(&ssign, '\0', sizeof(ssign));

    for(countchar=0; countchar<5; countchar++)      //Viene assegnato '-' ad ogni carattere di ssing
        ssign[countchar] = '-';

    countchar = 0;

    for(posbuffer = 0; posbuffer < 5; posbuffer++)      //Ciclo con algoritmo ottimizzato per confrontare le parole e scrivere il risultato in una stringa
    {
        posword = 0;

        while(posword < 5)
        {
            if(posword < posbuffer)
            {
                if(mainbuffer[posbuffer+5] == word[posword] || abs(mainbuffer[posbuffer+5] - word[posword]) == 32)
                {
                    ssign[posbuffer] = '+';
                    posword = posbuffer;
                }
                else
                    posword++;
            }
            else
            {
                if(posword == posbuffer)
                {
                    if(mainbuffer[posbuffer+5] == word[posword] || abs(mainbuffer[posbuffer+5] - word[posword]) == 32)
                    {
                        ssign[posbuffer] = '*';
                        posword = 5;
                        countchar++;
                    }
                    else
                    {
                        if(ssign[posbuffer] == '+')
                            posword = 5;
                        else
                            posword++;
                    }
                }
                else
                {
                    if(mainbuffer[posbuffer+5] == word[posword] || abs(mainbuffer[posbuffer+5] - word[posword]) == 32)
                    {
                        ssign[posbuffer] = '+';
                        posword = 5;
                    }
                    else
                        posword++;
                }     
            }
        }
    }

    memset(&buffer, '\0', sizeof(buffer));
    fprintf(stdout, "Stringa generata: %s\n", ssign);

    if(countchar == 5)      //Controllo se tutti i caratteri sono uguali
    {
        sprintf(buffer, "OK PERFECT\n");
        fprintf(stdout, "\nParola indovinata.\n\nMessaggio finale: %sNumero caratteri: %lu + \\n.\n", buffer, strlen(buffer) - 1);
        
        endsignal = 1;
    }
    else
    {
        if(ntry != maxtry)      //Controllo se i tentativi sono terminati
        {
            sprintf(buffer, "OK %d %s\n", ntry, ssign);
            fprintf(stdout, "\nMessaggio di risposta: %sNumero caratteri: %lu + \\n.\n", buffer, strlen(buffer) - 1);
        }   
        else
        {
            sprintf(buffer, "END %d %s\n", ntry, word);
            fprintf(stdout, "\nTentativi terminati.\n\nMessaggio finale: %sNumero caratteri: %lu + \\n.\n", buffer, strlen(buffer) - 1);

            endsignal = 1;
        }
    }

    returnStatus = write(simpleChildSocket, buffer, strlen(buffer));        //Invia il messaggio al server
        
    if (returnStatus <= 0)      //Controllo la connessione con il client
    {
        fprintf(stderr, "\nErrore: invio messaggio fallito, impossibile stabilire una connessione con il client.\n");
        endsignal = 1;
    }
    else
        fprintf(stdout, "\nMessaggio inviato.\n");

    return endsignal;
}