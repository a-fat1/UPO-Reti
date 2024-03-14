#include <arpa/inet.h>
#include <ctype.h>
#include <float.h>
#include <locale.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define MAX_LENGTH_BUFFER 513

void check_args(int argc, char **argv);
void print_localtime(char *buffer, int length_buffer);
int process_input(char *input, int *total_data_count, double *mean, double *m2);
void read_write(int *returnStatus, int simpleChildSocket);
int valid_input(char *input);

int main(int argc, char* argv[]) {
    // Inizializzazione delle strutture e delle variabili
    struct sockaddr_in simpleServer;
    struct sockaddr_in clientName = {0};
    socklen_t clientNameLength;
    int simpleSocket, simpleChildSocket, simplePort, returnStatus;

    check_args(argc, argv); // Controllo degli argomenti passati al programma

    simpleSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);   // Creazione del socket
    if(simpleSocket == -1) {    // Controllo la creazione del socket, in caso di errore termino il programma
        fprintf(stderr, "\n\nErrore: impossibile creare il socket.\n\n\n");
        exit(EXIT_FAILURE);
    }

    simplePort = atoi(argv[1]); // Conversione della porta da stringa a intero
    memset(&simpleServer, 0, sizeof(simpleServer)); // Reset della struttura del server

    // Configurazione della struttura del server
    simpleServer.sin_family = AF_INET;  
    simpleServer.sin_addr.s_addr = htonl(INADDR_ANY);
    simpleServer.sin_port = htons(simplePort);

    returnStatus = bind(simpleSocket, (struct sockaddr*)&simpleServer, sizeof(simpleServer));   // Collegamento del socket al indirizzo e alla porta specificati
    if(returnStatus == -1) {    // Controllo l'esecuzione del bind, in caso di errore termino il programma
        fprintf(stderr, "\n\nErrore: impossibile eseguire il bind.\nNota bene: dopo la chiusura di un precedente programma server, bisogna attendere un po' di tempo prima di avviare un altro programma simile.\n\n\n");
        close(simpleSocket);
        exit(EXIT_FAILURE);
    }

    returnStatus = listen(simpleSocket, 5); // Ascolta sul socket per eventuali connessioni
    if(returnStatus == -1) {    // Controllo l'avvio dell'ascolto, in caso di errore termino il programma
        fprintf(stderr, "\n\nErrore: impossibile ascoltare le richieste di connessione.\n\n\n");
        close(simpleSocket);
        exit(EXIT_FAILURE);
    }

    print_localtime(NULL, 0);   // Stampa dell'ora locale

    while(1) {  // Loop infinito per gestire le connessioni
        clientNameLength = sizeof(clientName);
        fprintf(stdout, "\n\n\nIn attesa di connessione...\n\n");

        simpleChildSocket = accept(simpleSocket, (struct sockaddr*)&clientName, &clientNameLength); // Accetta la connessione con un client
        if(simpleChildSocket == -1) {   // Controllo sulla connessione, in caso di errore termino il programma
            fprintf(stderr, "\n\nErrore: impossibile accettare le connesioni.\n\n\n");
            close(simpleSocket);
            exit(EXIT_FAILURE);
        }

        fprintf(stdout, "\n\nConnessione effetuata.\n\n");
        read_write(&returnStatus, simpleChildSocket);   // Legge e scrive dal/verso il socket
        
        fprintf(stdout, "\n\nConnessione terminata.\n");
        close(simpleChildSocket);   // Chiude la la connessione con il client
    }

    // Questo punto del codice non dovrebbe mai essere raggiunto poiché il server dovrebbe essere sempre in esecuzione
    // Pertanto, non serve chiudere il socket o terminare il programma con un codice di uscita
}

// Controllo degli argomenti passati da linea di comando
void check_args(int argc, char **argv) {
    if (argc != 2) { // Controllo il numero di argomenti, in caso di errore stampo un messaggio e termino il programma
        fprintf(stderr, "\n\nPer avviare il programma sono necessari 2 argomenti: <nome programma> <porta>\n\n<nome programma> equivale a: %s\n<porta> corrisponde ad un valore numerico compreso tra 1024 e 65535\n<numero tentativo>\n\nGli argomenti, inoltre, devono essere separati fra di loro con degli spazi.\n\n\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char* port_str = argv[1];   // Ottieni la stringa del numero di porta dall'argomento della linea di comando

    for (long unsigned int checkarg = 0; checkarg < strlen(port_str); checkarg++)   // Controllo che la stringa della porta contenga solo cifre, in caso di errore stampo un messaggio e termino il programma
        if(isdigit(port_str[checkarg]) == 0) {
            fprintf(stderr, "\n\nErrore: il valore della porta deve essere esclusivamente numerico.\n\n\n");
            exit(EXIT_FAILURE);
        }

    if (atoi(port_str) < 1024 || atoi(port_str) > 65535) {   // Controlla il range della porta (1024-65535), in caso di errore stampo un messaggio e termino il programma
        fprintf(stderr, "\n\nErrore: il numero della porta deve essere compreso tra i valori 1024 e 65535.\n\n\n");
        exit(EXIT_FAILURE);
    }
}

// Stampa a schermo (o salva in un buffer) il tempo locale
void print_localtime(char *buffer, int length_buffer) {
    time_t ticks = time(NULL);  // Ottiene il tempo corrente in secondi
    struct tm *tm = localtime(&ticks);  // Converte il tempo corrente in una struttura tm locale
    char date[MAX_LENGTH_BUFFER];   // Buffer locale per la data e l'ora

    setlocale(LC_TIME, "it_IT.utf8");   // Imposta la localizzazione del tempo in italiano
    memset(date, '\0', MAX_LENGTH_BUFFER);  // Inizializza il buffer 'date' con tutti i caratteri nulli

    if (buffer == NULL && length_buffer == 0) {
        strftime(date, sizeof(date), "%A %d %B %Y, ore %H:%M:%S", tm);  // Formatta la data e l'ora nella stringa 'date'
        fprintf(stdout, "\n\nServer inizializzato con successo in data: %s.\n", date);  // Stampa a schermo la data e l'ora
    } else {
        strftime(buffer, length_buffer, "OK START Benvenuto/a. Data e ora di connessione (riferita al server): %A %d %B %Y, %H:%M:%S.\n", tm);  // Formatta la data e l'ora nel buffer in input
        fprintf(stdout, "\nMessaggio OK START generato."); // Stampa a schermo il buffer
    }
}

// Funzione che definisce un ciclo di ricezione-invio dei messaggi
void read_write(int *returnStatus, int simpleChildSocket) {
    // Inizializzazione delle variabili
    char buffer[MAX_LENGTH_BUFFER];
    long unsigned int v_supp = 1;
    int check, total_data_count = 0;
    double mean = 0.0, m2 = 0.0;
    
    *returnStatus = 0;  // Inizializzazione del valore puntato da returnStatus a 0

    do {
        memset(&buffer, '\0', MAX_LENGTH_BUFFER);   // Pulizia del buffer prima di ogni lettura
        if(v_supp) {    // La prima volta si invia il messaggio di benvenuto, quindi si salta la lettura sul socket
            print_localtime(buffer, MAX_LENGTH_BUFFER);
            v_supp = 0;
        } else {    // Altrimenti si legge dal socket e si gestiscono i dati
            fprintf(stdout, "\n\nIn attesa di risposta...\n\n");

            check = read(simpleChildSocket, buffer, MAX_LENGTH_BUFFER); // Lettura dei dati dal socket
            if (check > 0) {   // Se la lettura sul socket ha successo si prosegue, altrimenti viene stampato un messaggio di errore
                fprintf(stdout, "\nMessaggio ricevuto. (%zu)\n", strlen(buffer));

                if (buffer[0] == '0') { // Se il primo carattere del buffer equivale a '0' significa che il programma deve terminare, altrimenti bisogna elaborare i dati
                    fprintf(stdout, "\nCarattere di terminazione: '%c'.\n", buffer[0]);
                    if (buffer[1] != '\n' || strlen(buffer) != 2) { // Se il messaggio non equivale esattamente a "0\n" bisogna stampare e inviare un messaggio di errore
                        memset(&buffer, '\0', MAX_LENGTH_BUFFER);
                        sprintf(buffer, "ERR SYNTAX Errore: quando si cerca di terminare il programma, il messaggio deve essere composto (solo) dai caratteri '0' e d'invio.\n");
                        fprintf(stderr, "Errore: struttura messaggio incorretta. (buffer != 2 e/o buffer[1] != '\\n')\nMessaggio di ERR SYNTAX generato.\n");
                    } else {    // Con il messaggio equivalente a "0\n" si elaborano le statistiche finali
                        fprintf(stdout, "Struttura messaggio: corretta.\n");
                        memset(&buffer, '\0', MAX_LENGTH_BUFFER);

                        if(total_data_count == 0) { // Nel caso di 0 dati in input stampa e invia il relativo messaggio di errore
                            sprintf(buffer, "ERR STATS Nessun dato inserito, quindi la media e la varianza non possono essere calcolate.\n");
                            fprintf(stderr, "0 dati inseriti, impossibile calcolare media e varianza.\nMessaggio di ERR STATS generato.\n");
                        }
                        else if (total_data_count == 1) {   // Nel caso di un solo dato in input stampa e invia il relativo messaggio di errore
                            sprintf(buffer, "ERR STATS Con un solo inserito la media equivale a quel dato (%.0f), mentre la varianza campionaria necessita almeno due dati.\n", mean);
                            fprintf(stderr, "1 dato inserito, impossibile calcolare la varianza.\nMessaggio di ERR STATS generato.\n");
                        }
                        else {
                            v_supp = snprintf(buffer, sizeof(buffer), "OK STATS %d %lf %lf\n", total_data_count, mean, m2 / (total_data_count - 1));
                            if (v_supp >= sizeof(buffer)) { // Controllo su media e varianza al fine di verificare che siano sufficientemente piccoli per il buffer
                                memset(&buffer, '\0', MAX_LENGTH_BUFFER);
                                sprintf(buffer, "ERR STATS Errore: impossibile inviare la media e la varianza, la lunghezza dei dati eccede la dimensione massima del messaggio.\n");
                                fprintf(stderr, "Errore: overflow messaggio finale.\nMessaggio di ERR STATS generato.\n");
                            }
                            else
                                fprintf(stdout, "Messaggio OK STATS generato. (%lu)\n", v_supp);
                        }
                    }

                    *returnStatus = 1;  // Imposta lo stato a 1 per terminare la connessione
                } else
                    *returnStatus = process_input(buffer, &total_data_count, &mean, &m2);
            } else {
                fprintf(stderr, "\nErrore: impossibile stabilire una connessione con il client.\n");
                *returnStatus = -1;
            }
        }

        if (*returnStatus != -1) {  // Se la lettura ha avuto successo si scrive la risposta sul socket
            check = write(simpleChildSocket, buffer, strlen(buffer));   // Scrittura dei dati sul socket

            if(check <= 0) {  // Se la scrittura fallisce, segnala l'errore e termina la connessione
                fprintf(stderr, "\nErrore: invio messaggio fallito, impossibile stabilire una connessione con il client.\n");
                *returnStatus = 1;
            } else  // Se la scrittura sul socket ha successo stampa un messaggio di conferma
                fprintf(stdout, "\nMessaggio inviato. (%zu)\n", strlen(buffer));
        } else  // In caso di esito negatico con read
            *returnStatus = 1;

    } while (*returnStatus != 1);   // Ripete read e write fino a quando returnStatus != 1
}

// Funzione per validare la semantica dell'input ricevuto dal client.
int process_input(char *input, int *total_data_count, double *mean, double *m2) {
    if (!valid_input(input))    // Controlla se valid_input restituisce 1. Se restituisce 0, ritorna 1
        return 1;
    else {   // Rimuove l'ultimo carattere '\n' dalla stringa di input
        fprintf(stdout, "\nStruttura sintattica del messaggio: corretta.\n");
        input[strlen(input) - 1] = '\0';
    }

    // Inizializza le variabili
    int num_data, data_count = 0;
    sscanf(input, "%d", &num_data); // Estrae il primo numero da input, che rappresenta il numero totale di numeri che seguiranno
    fprintf(stdout, "\nIdentificazione dei dati\nPrimo numero del messaggio ricevuto: %d\n", num_data);

    char maxDbl[MAX_LENGTH_BUFFER]; // Calcola la lunghezza del numero massimo rappresentabile con un double e lo memorizza in maxDblLength
    snprintf(maxDbl, MAX_LENGTH_BUFFER, "%.0lf", DBL_MAX);
    int maxDblLength = strlen(maxDbl);

    char *token = strtok(input, " ");   // Divide il messaggio in input in pezzi (token) attraverso gli spazi fra i dati
    token = strtok(NULL, " ");

    double temp_data;
    while (token != NULL) {
        int tokenLength = strlen(token);
        
        if (tokenLength > maxDblLength) {   // Controlla se la lunghezza del token supera quella del numero massimo rappresentabile con un double
            memset(input, '\0', MAX_LENGTH_BUFFER);
            sprintf(input, "ERR DATA Errore: uno dei numeri inseriti supera il valore massimo memorizzabile.\n");
            fprintf(stderr, "Dato %d: errore\n\nIl dato %d supera valore massimo double.\nMessaggio di ERR DATA generato.\n", data_count + 1, data_count + 1);
            return 1;
        } else if (tokenLength == maxDblLength) {   // Controlla se il token ha la stessa lunghezza del numero massimo rappresentabile con un double
            for (int i = 0; i < tokenLength; i++) {
                if (token[i] > maxDbl[i]) {
                    memset(input, '\0', MAX_LENGTH_BUFFER);
                    sprintf(input, "ERR DATA Errore: uno dei numeri inseriti supera il valore massimo memorizzabile.\n");
                    fprintf(stderr, "Dato %d: errore\n\nIl dato %d supera valore massimo double.\nMessaggio di ERR DATA generato.\n", data_count + 1, data_count + 1);
                    return 1;
                } else if (token[i] < maxDbl[i])
                    break;
            }
        }
    
        sscanf(token, "%lf", &temp_data);   // Converte il token (stringa) in un double
        fprintf(stdout, "Dato %d: %.0lf (%d)\n", data_count + 1, temp_data, tokenLength);
        data_count++;

        // Algoritmo di Knuth (basato sulla formula di Welford) per calcolare media e varianza (e controlli overflow/underflow)
        if ((temp_data > 0.0 && *mean > DBL_MAX + temp_data) || (temp_data < 0.0 && *mean < DBL_MIN + temp_data)) {     // Controllo per temp_data - *mean, stampa e invia un messaggio di errore nel caso di esito positivo
            memset(input, '\0', MAX_LENGTH_BUFFER);
            sprintf(input, "ERR DATA Errore: rilevato overflow/underflow nella differenza tra un dato e la media.\n");
            fprintf(stderr, "\nErrore: differenza 1 --> Overflow/underflow.\nMessaggio di ERR DATA generato.\n");
            return 1;
        }
        double delta = temp_data - *mean;
        if ((*mean > 0.0 && delta > DBL_MAX - *mean * (*total_data_count + data_count)) || (*mean < 0.0 && delta < DBL_MIN - *mean * (*total_data_count + data_count))) {       // Controllo per *mean + delta / (*total_data_count + data_count), stampa e invia un messaggio di errore nel caso di esito positivo
            memset(input, '\0', MAX_LENGTH_BUFFER);
            sprintf(input, "ERR DATA Errore: rilevato overflow/underflow nel calcolo della media.\n");
            fprintf(stderr, "\nErrore: media --> Overflow/underflow.\nMessaggio di ERR DATA generato.\n");
            return 1;
        }
        *mean += delta / (*total_data_count + data_count);
        if ((temp_data > 0.0 && *mean > DBL_MAX + temp_data) || (temp_data < 0.0 && *mean < DBL_MIN + temp_data)) {     // Controllo per temp_data - *mean, stampa e invia un messaggio di errore nel caso di esito positivo
            memset(input, '\0', MAX_LENGTH_BUFFER);
            sprintf(input, "ERR DATA Errore: rilevato overflow/underflow nella differenza per calcolare la varianza campionaria.\n");
            fprintf(stderr, "\nErrore: differenza 2 --> Overflow/underflow.\nMessaggio di ERR DATA generato.\n");
            return 1;
        }
        double delta2 = temp_data - *mean;
        if ((*m2 > 0.0 && delta * delta2 > DBL_MAX - *m2) || (*m2 < 0.0 && delta * delta2 < DBL_MIN - *m2)) {       // Controllo per *m2 + delta * delta2, stampa e invia un messaggio di errore nel caso di esito positivo
            memset(input, '\0', MAX_LENGTH_BUFFER);
            sprintf(input, "ERR DATA Errore: rilevato overflow/underflow nel calcolo della varianza campionaria.\n");
            fprintf(stderr, "\nErrore: varianza --> Overflow/underflow.\nMessaggio di ERR DATA generato.\n");
            return 1;
        }
        *m2 += delta * delta2;

        token = strtok(NULL, " ");  // Passa al prossimo token
    }

    memset(input, '\0', MAX_LENGTH_BUFFER);

    if (data_count != num_data) {   // Controlla se il numero di dati elaborati corrisponde a quello previsto
        sprintf(input, "ERR DATA Errore: Il numero dei dati inserito non corrisponde ai dati presenti.\n");
        fprintf(stderr, "\nErrore: numero dati ricevuto errato.\nMessaggio di ERR DATA generato.\n");
        return 1;
    } else
        *total_data_count += data_count;

    sprintf(input, "OK DATA %d\n", num_data);   // Messaggio da passare al client e, successivamente, i messaggi stampati a terminale per confermare l'esito positivo sul salvataggio dei dati

    fprintf(stdout, "\nStruttura semantica confermata, dati salvati correttamente.\nMessaggio OK DATA generato.\n\nDati inseriti: %d\nNumero totale dei dati: %d\nMedia e varianza campionaria attuali:\n%lf ", num_data, *total_data_count,  *mean);
    if(*total_data_count - 1 == 0)
        fprintf(stdout, "Non calcolabile\n");
    else
        fprintf(stdout, "%lf\n", *m2 / (*total_data_count - 1));

    return 0;
}

// Funzione per validare la sintassi dell'input ricevuto dal client
int valid_input(char *input) {
    size_t length = strlen(input); // Calcola la lunghezza del messaggio in input

    if (length < 4 || input[length - 1] != '\n') {    // Controlla la lunghezza minima del messaggio e se l'ultimo carattere non equivale all'invio; in quel caso stampa e invia il messaggio di errore specifico
        memset(input, '\0', MAX_LENGTH_BUFFER);
        sprintf(input, "ERR SYNTAX Errore: il messaggio deve contenere un numero di caratteri compreso tra 4 e 512, e deve terminare con il carattere d'invio.\n");
        fprintf(stderr, "\nErrore: formato messaggio errato. (mex < 4 o mex > 512 e/o mex[l - 1] != '\\n')\nMessaggio di ERR SYNTAX generato.\n");
        return 0;
    }
    if (!isdigit(input[0]) || !isdigit(input[length - 2])) {   // Controlla se il primo ed il penultimo carattere dell'input siano dei numeri
        if (input[0] == ' ' || input[length - 2] == ' ') {  // Se il messaggio inizia o termina con uno spazio, stampa e invia il relativo messaggio di errore
            memset(input, '\0', MAX_LENGTH_BUFFER);
            sprintf(input, "ERR SYNTAX Errore: gli spazi non sono ammessi all'inizio e alla fine del messaggio (prima dell'invio).\n");
            fprintf(stderr, "\nErrore: il messaggio comincia e/o finisce con ' '.\nMessaggio di ERR SYNTAX generato.\n");
        } else {    // In caso contrario stampa e invia un messaggio per i caratteri non consentiti
            memset(input, '\0', MAX_LENGTH_BUFFER);
            sprintf(input, "ERR SYNTAX Errore: non sono ammessi caratteri diversi da quelli numerici, soprattutto all'inizio e alla fine del messaggio (prima dell'invio).\n");
            fprintf(stderr, "\nErrore: il messaggio comincia e/o si conclude con un carattere non numerico.\nMessaggio di ERR SYNTAX generato.\n");
        }
        return 0;
    }
    int space_count = 0;
    for (size_t i = 0; i < length - 1; i++) {   // Conta gli spazi nel messaggio e verifica che dopo ogni spazio sia presente un carattere numerico
        if (input[i] == ' ') {
            space_count++;
            if (i + 1 < length - 1 && !isdigit(input[i + 1])) {    // Controlla che ogni carattere successivo al carattere spazio sia esclusivamente un carattere numerico
                if (input[i + 1] == ' ') {  // Nel caso del doppio spazio, stampa e invia un messsaggio di errore specifico
                    memset(input, '\0', MAX_LENGTH_BUFFER);
                    sprintf(input, "ERR SYNTAX Errore: per la separazione dei dati deve essere presente solo un carattere di spazio.\n");
                    fprintf(stderr, "\nErrore: doppio spazio all'interno del messaggio.\nMessaggio di ERR SYNTAX generato.\n");
                } else {    // Altrimenti stampa e invia un messaggio per i caratteri non consentiti
                    memset(input, '\0', MAX_LENGTH_BUFFER);
                    sprintf(input, "ERR SYNTAX Errore: dopo ogni spazio deve esserci esclusivamente un carattere numerico.\n");
                    fprintf(stderr, "\nErrore: carattere non numerico successivo allo spazio.\nMessaggio di ERR SYNTAX generato.\n");
                }
                return 0;
            }
        } else if (!isdigit(input[i])) {    // Controlla che il carattere sia solo una cifra o il carattere d'invio (\n)
            memset(input, '\0', MAX_LENGTH_BUFFER);
            sprintf(input, "ERR SYNTAX Errore: il messaggio deve contenere solo spazi e caratteri numerici.\n");
            fprintf(stderr, "\nErrore: carattere non consentito nel messaggio.\nMessaggio di ERR SYNTAX generato.\n");
            return 0;
        }
    }
    if (space_count < 1) {  // Verifica che ci sia almeno uno spazio nel messaggio per distinguere i dati dal numero dei dati
        memset(input, '\0', MAX_LENGTH_BUFFER);
        sprintf(input, "ERR SYNTAX il messaggio necessita dei caratteri di spazio al fine di identificare il numero dei dati e i dati stessi.\n");
        fprintf(stderr, "\nErrore: nessuno spazio nel messaggio.\nMessaggio di ERR SYNTAX generato.\n");
        return 0;
    }

    return 1;   // Se il messaggio passa tutti i controlli, ritorna 1
}