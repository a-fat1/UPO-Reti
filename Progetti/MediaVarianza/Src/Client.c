#include <arpa/inet.h>
#include <ctype.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_LENGTH_BUFFER 513

void check_args(int argc, char **argv);
int check_message_structure(const char *input);
void parse_and_print_message(const char *input, int *returnStatus, int *num_data, int *total_data_count);
int valid_input(char *input, int *num_data, int *returnStatus);
void write_read(int *returnStatus, int simpleSocket);

int main(int argc, char* argv[]) {
    // Inizializzazione della struttura e delle variabili
    struct sockaddr_in simpleServer;
    int simpleSocket, returnStatus = 0;

    check_args(argc, argv); // Verifica della correttezza degli argomenti passati al programma

    simpleSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);   // Creazione del socket
    if(simpleSocket == -1) {    // Controllo esito creazione del socket
        fprintf(stderr, "\n\nErrore: impossibile creare il socket.\n\n\n");
        exit(EXIT_FAILURE);
    }
    memset(&simpleServer, 0, sizeof(simpleServer)); // Imposta a 0 i valori della struttura simpleServer

    simpleServer.sin_family = AF_INET;  // Impostazione del tipo di indirizzo
    simpleServer.sin_addr.s_addr = inet_addr(argv[1]);  // Impostazione dell'indirizzo IP del server
    simpleServer.sin_port = htons(atoi(argv[2]));   // Impostazione della porta del server

    fprintf(stdout, "\n\nAvviso: se il programma non dovesse rispondere, comunemente le cause sono attribuite al programma server a cui ci si collega.\nInfatti, un server che non riesce a gestire adeguatamente i messaggi e/o le connessioni provoca un tempo d'attesa (definito o infinito) nel programma client.\n\n\n");

    returnStatus = connect(simpleSocket, (struct sockaddr*)&simpleServer, sizeof(simpleServer));    // Tentativo di connessione al server
    if(returnStatus == 0) { // Controllo esito della connessione
        write_read(&returnStatus, simpleSocket);    // Se la connessione ha esito positivo, invia e riceve messaggi dal server
        close(simpleSocket);    // Chiusura del socket
    } else {    // In caso di fallimento stampa un messaggio di errore e termina il programma
        fprintf(stderr, "Errore: impossibile connettersi all'indirizzo IP.\nIl server potrebbe essere non raggiungibile oppure l'indirizzo IP e/o la porta potrebbero essere errati.\n\n\n");
        close(simpleSocket);
        exit(EXIT_FAILURE);
    }

    return(EXIT_SUCCESS);   // Se le operazioni hanno avuto esito positivo, il programma termina con successo
}

// Controllo degli argomenti in input
void check_args(int argc, char **argv) {
    // Inizializzazione delle variabili
    long unsigned int checkarg;
    char *arg = argv[2];

    if(argc == 3) { // Controllo sul numero di argomenti, in caso di esito negativo stampa un messaggio di errore e termina il programma
        for(checkarg = 0; checkarg < strlen(arg); checkarg++)    // Controlla se tutti i caratteri della porta sono numerici
            if(isdigit(arg[checkarg]) == 0) {
                fprintf(stderr, "\n\nErrore: i caratteri della porta devono essere esclusivamente numerici.\n\n\n");    // Messaggio di errore e terminazione del programma
                exit(EXIT_FAILURE);
            }

        if(atoi(arg) < 1024 || atoi(arg) > 65535) { // Controlla il range della porta (1024-65535), in caso di errore stampa il seguente messaggio a terminale e termina il programma
            fprintf(stderr, "\n\nErrore: il numero della porta deve essere compreso tra i valori 1024 e 65535.\n\n\n");
            exit(EXIT_FAILURE);
        }
    } else {
        fprintf(stderr, "\n\nPer avviare il programma sono necessari 3 argomenti: <nome programma> <IPv4 server> <porta server>\n\n<nome programma> equivale a: %s\n<IPv4 server> equivale all'indirizzo IP del programma server a cui ci si vuole collegare (Es. 81.22.36.245)\n<porta server> corrisponde alla porta del server che possiede un valore numerico compreso tra 1024 e 65535\n\nGli argomenti, inoltre, devono essere separati fra di loro con degli spazi.\n\n\n", argv[0]);
        exit(EXIT_FAILURE);
    }
}

// Funzione che definisce un ciclo di invio-ricezione dei messaggi
void write_read(int *returnStatus, int simpleSocket) {
    // Inizializzazione delle variabili
    int check, num_data, total_data_count = 0, repeat = 1, first_time = 1;
    char buffer[MAX_LENGTH_BUFFER];

    do {
        if(!first_time) {   // La prima volta si riceve il messaggio di benvenuto, quindi si salta la scrittura sul socket
            fprintf(stdout ,"\nInserisci qui la sequenza di dati o digita '0' per terminare: ");
            do {
                memset(&buffer, '\0', MAX_LENGTH_BUFFER);   // Azzera il buffer e legge l'input dall'utente
                fgets(buffer, MAX_LENGTH_BUFFER, stdin);

                if(strlen(buffer) == MAX_LENGTH_BUFFER - 1 && buffer[MAX_LENGTH_BUFFER - 2] != '\n') {  // Controlla se l'input rispetta la lunghezza massima del messaggio
                    fprintf(stderr, "Hai inserito una sequenza di dati troppo lunga. Le dimensioni massime consentite sono di %d caratteri.\n", MAX_LENGTH_BUFFER - 1);
                    while((check = getchar()) != '\n' && check != EOF);   // Rimuove i caratteri d'input in eccesso
                    fprintf(stdout ,"\nPer favore inserisci qui il messaggio corretto: ");
                } else {
                    repeat = valid_input(buffer, &num_data, returnStatus);  // Controlla l'input dell'utente
                    if(repeat == 0) {
                        check = write(simpleSocket, buffer, strlen(buffer));    // Invia l'input validato al server

                        if(check <= 0) {    // In caso di errore il flag di terminazione viene impostato a -1 per evitare la funzione read
                            fprintf(stderr, "Errore: invio messaggio fallito, impossibile stabilire una connessione con il server.\n\n\n");
                            *returnStatus = -1;
                        }
                    } else
                        fprintf(stdout ,"\nPer favore inserisci qui il messaggio corretto: ");
                }
            } while(repeat != 0);
        } else
            first_time = 0;

        if(*returnStatus != -1) {   // In assenza di errore legge la risposta del server, altrimenti imposta il flag di terminazione a 1 per terminare il programma
            memset(&buffer, '\0', MAX_LENGTH_BUFFER);   // Azzera il buffer e legge il messaggio dal server
            check = read(simpleSocket, buffer, MAX_LENGTH_BUFFER);
            
            if(check > 0)   // Verifica di aver ricevuto il messaggio, altrimenti stampa un messaggio di errore
                parse_and_print_message(buffer, returnStatus, &num_data, &total_data_count);    // Analizza e stampa la risposta del server
            else {
                fprintf(stderr, "Errore: impossibile stabilire una connessione con il server.\n\n\n");
                *returnStatus = 1;
            }
        } else
            *returnStatus = 1;

    } while (*returnStatus != 1);   // Ripete write e read fino a quando returnStatus != 1
}

// Controlla la struttura dei dati da inviare
int valid_input(char *input, int *num_data, int *returnStatus) {
	size_t length = strlen(input); // Calcola la lunghezza dell'input

	if (input[0] == '0') {  // Controlla se l'input equivale a '0'
		if (input[1] != '\n' || length != 2) { // Verifica che messaggio di terminazione rispetti il formato corretto
	        fprintf(stderr, "Errore: quando si inserisce '0' come primo carattere significa che si sta cercando di terminare l'applicazione.\nDi conseguenza, per terminare il programma correttamente bisogna digitare solo il carattere '0' e premere invio.\n");
	    	return 1;
		} else {    // In caso di esito positivo, imposta il flag di terminazione e ritorna 0 per impostare repeat nella funzione write_read
	        *returnStatus = 1;
	        return 0;
		}
	} else {
        if (length < 4 || input[length - 1] != '\n') {    // Controlla la lunghezza minima dell'input e se termina con l'invio (\n)
			fprintf(stderr, "Errore: il messaggio deve avere come minimo 4 caratteri, e deve terminare con l'invio.\n");
			return 1;
		}
        if (!isdigit(input[0]) || !isdigit(input[length - 2])) {   // Verifica che il primo ed il penultimo carattere dell'input siano dei numeri
        	if (input[0] == ' ' || input[length - 2] == ' ') {   // Stampa messaggio di errore specifico per lo spazio, altrimenti stampa l'errore per gli altri caratteri
        	    fprintf(stderr, "Errore: gli spazi non sono ammessi all'inizio e alla fine del messaggio (prima dell'invio).\n");
            } else {
                fprintf(stderr, "Errore: non sono ammessi caratteri diversi da quelli numerici, soprattutto all'inizio e alla fine del messaggio (prima dell'invio).\n");
            }
        	return 1;
    	}
    	int space_count = 0;
    	for (size_t i = 0; i < length - 1; i++) {  // Conta gli spazi nel messaggio e verifica che dopo ogni spazio sia presente un carattere numerico
    	    if (input[i] == ' ') {
    	        space_count++;

    		    if (i + 1 < length - 1 && !isdigit(input[i + 1])) {    // Controlla che ogni carattere successivo al carattere spazio sia esclusivamente un carattere numerico
    		        if (input[i + 1] == ' ') {  // Nel caso del doppio spazio, stampa e invia un messsaggio di errore specifico
    		            fprintf(stderr, "Errore: per la separazione dei dati deve essere presente solo un carattere di spazio.\n");
                    } else {    // Altrimenti stampa e invia un messaggio per i caratteri non consentiti
                        fprintf(stderr, "Errore: dopo ogni spazio deve esserci esclusivamente un carattere numerico.\n");
                    }
    		        return 1;
    		    }
    		} else if (!isdigit(input[i])) {   // Controlla che il carattere sia solo una cifra
			    fprintf(stderr, "Errore: il messaggio deve contenere solo spazi e caratteri numerici.\n");
        	    return 1;
        	}
    	}
    	if (space_count < 1) {  // Verifica che nell'input ci sia almeno uno spazio per distinguere i dati dal numero dei dati
    	    fprintf(stderr, "Errore: il messaggio deve contenere degli spazi al fine di identificare correttamente il numero dei dati e i dati stessi.\n");
    	    return 1;
    	} else {    // Controlla se il numero di spazi contati corrisponde a quello indicato nell'input
    		sscanf(input, "%d", num_data);
    		if (space_count != *num_data) {
    			fprintf(stderr, "Errore: il numero dei dati non corrisponde.\n\nDati dichiarati: %d.\nDati inseriti: %d.\n", *num_data, space_count);
        		return 1;
    		}
    	}
	}
	
	return 0;   // Se l'input passa tutti i controlli, ritorna 0
}

// Analizza e stampa la risposta del server
void parse_and_print_message(const char *input, int *returnStatus, int *num_data, int *total_data_count) {
    if (!check_message_structure(input)) {  // Verifica la struttura della messaggio, in caso di anomalie imposta il flag di terminazione a 1
        *returnStatus = 1;
        return;
    }

    // Inizializza variabili per l'analisi del messaggio
    char message_type[5], message_subtype[10];
    int number;

    int scan_result = sscanf(input, "%s %s", message_type, message_subtype);    // Legge l'esito e il tipo di risposta del messaggio
    if (scan_result != 2) { // Se non si riesce ad identificare le prime due parti del messaggio, imposta il flag di terminazione a 1 
        fprintf(stderr, "Errore: impossibile identificare la presenza dell'esito e/o del tipo di risposta ricevuta dal server.\n\n\n");
        *returnStatus = 1;
        return;
    }

    // Inizializza stringa e puntatore per la ricerca nel messaggio
    char search_string[MAX_LENGTH_BUFFER];
    const char *start_of_message;

    if (strcmp(message_type, "OK") == 0) {  // Gestisce il caso in cui l'esito equivale a "OK"
        sprintf(search_string, "OK %s ", message_subtype);  // Costruisce la stringa di ricerca
        start_of_message = strstr(input, search_string);    // Cerca la stringa di ricerca nel messaggio e ritorna un puntatore
        if (start_of_message != NULL) { // Nel caso la ricerca della stringa riscontri successo prosegue con le operazioni, altrimenti stampa un errore e imposta flag di terminazione a 1
            if (strcmp(message_subtype, "START") == 0) {    // Gestisce il caso in cui il tipo di risposta equivale a START
                start_of_message += strlen(search_string);    // Avanza start_of_message per la lunghezza della stringa di ricerca
                fprintf(stdout, "Messaggio del server:\n\n%s\n\n", start_of_message);   // Stampa il messaggio del server e le istruzioni per l'utente
                fprintf(stdout, "Questo programma permette di calcolare la media e la varianza campionaria di un insieme di numeri.\nI seguenti punti ti illustreranno come utilizzare correttamente l'applicazione:\n\n");
                fprintf(stdout, "1 - Dopo queste istruzioni, sarai invitato ad inserire un insieme di numeri interi positivi per il calcolo della media e della varianza;\n");
                fprintf(stdout, "2 - Inserisci i numeri uno ad uno, separati da un singolo spazio, e dopo aver digitato l'ultimo numero premi invio;\n");
                fprintf(stdout, "3 - Il primo numero inserito deve rappresentare il numero di dati che vuoi inviare al server (esempio: se vuoi calcolare la media e la varianza sui numeri 10 20 12, per inviarli al server devi scrivere 3 10 20 12 e 12 deve terminare con un invio finale);\n");
                fprintf(stdout, "4 - Un messaggio ha dimensioni massime di 512 caratteri, quindi se vuoi inserire un numero elevato di dati devi farlo utilizzando molteplici messaggi;\n");
                fprintf(stdout, "5 - Se inserisci i dati nel formato corretto, successivamente essi vengono inviati al server per l'elaborazione. Una volta che vuoi calcolare la media e la varianza, inserisci solo il carattere '0' e premi invio;\n");
                fprintf(stdout, "6 - Dopo aver inviato il carattere '0', il server invia la media e la varianza campionaria solo se sono stati inseriti un numero di dati pari o maggiore a 2. Successivamente la connessione viene terminata.\n\n");
                fprintf(stdout, "Nota bene: in alcuni casi, quando commetti un errore, potrai immettere nuovamente il messaggio; ma se il client o il server ricevono un messaggio errato la connessione viene interrotta.\n");
                fprintf(stdout, "Inoltre, si sconsiglia di inserire dati troppo grandi, in quanto il server potrebbe non essere in grado di gestirli correttamente.\nSpero che la spiegazione sull'utilizzo di questo programma sia stata esaustiva.\n\n");
            } else if (strcmp(message_subtype, "DATA") == 0) {    // Gestisce il caso in cui il tipo di risposta equivale a DATA
                scan_result = sscanf(input, "OK DATA %d", &number); // Legge il numero dei dati inseriti nel messaggio di risposta
                if (scan_result == 1) { // Se il numero di dati equivale a 1 prosegue con le operazioni, altrimenti imposta flag di terminazione a 1
                    if (number == *num_data) {  // Se il numero di dati corrisponde a quello inserito dall'utente stampa il messaggio di conferma, altrimenti imposta flag di terminazione a 1
                        *total_data_count += *num_data;
                        if (number == 1)    // Stampa un messaggio di conferma, in base al numero dei dati
                            fprintf(stdout, "Il server ha memorizzato correttamente %d dato.\n", number);
                        else
                            fprintf(stdout, "Il server ha memorizzato correttamente %d dati.\n", number);
                    } else {
                        fprintf(stderr, "Errore: il numero di dati ricevuto dal server non corrisponde a quello inserito.\n\n\n");
                        *returnStatus = 1;
                    }
                } else {
                    fprintf(stderr, "Errore: impossibile identificare, nel contenuto del messaggio, il numero dei dati che sono stati inseriti.\n\n\n");
                    *returnStatus = 1;
                }
            } else if (strcmp(message_subtype, "STATS") == 0) {   // Gestisce il caso in cui il tipo di risposta equivale a STATS
                // Inizializzazione variabili per media e varianza
                double mean, sample_variance;
                scan_result = sscanf(input, "OK STATS %d %lf %lf", &number, &mean, &sample_variance); // Legge i dati ricevuti dal messaggio di risposta
                if (number < 2 || scan_result == 3) {   // Se le condizioni sono verificate prosegui con le verifiche, altrimenti stampa errore e imposta flag di terminazione a 1
                    if (number == *total_data_count) {  // Se il numero di dati corrisponde al conteggio totale prosegui con le verifiche, altrimenti stampa errore e imposta flag di terminazione a 1
                        if ((mean < 0 || sample_variance < 0) || (mean == 0 && sample_variance > 0)) {  // Se la media e la varianza non sono valide stampa errore e imposta flag di terminazione a 1
                            fprintf(stderr, "Errore: la media e/o la varianza ricevuta contengono dei valori non consentiti.\n\n\n");
                            *returnStatus = 1;
                        } else {  // Stampa il contenuto del messaggio ricevuti (media e varianza con 3 numeri decimali)
                            fprintf(stdout, "\nNumero di dati totali inseriti: %d\n", number);
                            fprintf(stdout, "Media: %.3lf\n", mean);
                            fprintf(stdout, "Varianza campionaria: %.3lf\n\n\n", sample_variance);
                        }
                    } else {
                        fprintf(stderr, "Errore: il numero totale dei dati inviati non corrisponde con quello ricevuto dal server.\n\n\n");
                        *returnStatus = 1;
                    }
                } else {
                    fprintf(stderr, "Errore: i dati ricevuti dal server sono errati e/o mancanti.\n\n\n");
                    *returnStatus = 1;
                }
            } else {    // Stampa errore e imposta flag di terminazione a 1 se il tipo di risposta non corrisponde
                fprintf(stderr, "Errore: impossibile identificare il tipo di risposta ricevuta dal server.\n\n\n");
                *returnStatus = 1;
            }
        } else {
            fprintf(stderr, "Errore: impossibile stabilire la struttura esito-tipo del messaggio ricevuto dal server.\n\n\n");
            *returnStatus = 1;
        }
    } else {
        if (strcmp(message_type, "ERR") == 0) { // Gestisce il caso in cui l'esito equivale a "ERR", oppure stampa un messaggio di errore
            if (strcmp(message_subtype, "DATA") == 0 || strcmp(message_subtype, "STATS") == 0 || strcmp(message_subtype, "SYNTAX") == 0) {  // Gestisce i casi in cui il tipo di risposta equivale a DATA, STATS o SYNTAX (oppure stampa un messaggio di errore)
                sprintf(search_string, "ERR %s ", message_subtype); // Costruisce la stringa di ricerca
                start_of_message = strstr(input, search_string);    // Cerca la stringa di ricerca nel messaggio
                if (start_of_message != NULL) { // Nel caso la ricerca della stringa riscontri successo prosegue con le operazioni, altrimenti stampa un messaggio di errore
                    start_of_message += strlen(search_string);  // Avanza start_of_message per la lunghezza della stringa di ricerca
                    fprintf(stderr, "%s\n\n", start_of_message);    // Stampa il messaggio di errore
                } else
                    fprintf(stderr, "Errore: impossibile stabilire la struttura esito-tipo del messaggio di errore.\n\n\n");
            } else
                fprintf(stderr, "Errore: impossibile identificare il tipo di errore ricevuto dal server.\n\n\n");
        } else
            fprintf(stderr, "Errore: impossibile identificare l'esito della risposta del server.\n\n\n");

        *returnStatus = 1;  // Imposta il flag di terminazione a 1
    }
}

// Verifica la struttura del messaggio ricevuto
int check_message_structure(const char *input) {
    size_t length = strlen(input); // Calcola la lunghezza del messaggio

    if (length < 10 || input[length - 1] != '\n') {   // Controlla la lunghezza del messaggio e se termina con l'invio (carattere '\n')
        fprintf(stderr, "Errore: la risposta del server non rispetta la struttura prestabilita.\nIl messaggio di risposta deve avere minimo 10 caratteri e massimo 512. Deve, inoltre, terminare con un invio finale.\nNormalmente, troppi caratteri in un messaggio causano l'omissione del carattere d'invio.\n\n\n");
        return 0;
    }
    if (strncmp(input, "OK", 2) != 0 && strncmp(input, "ERR", 3) != 0) {    // Controlla se il messaggio comincia con "OK" o "ERR"
        fprintf(stderr, "Errore: nel messaggio del server i primi caratteri sono riservati per identificare l'esito della risposta.\n\n\n");
        return 0;
    }
    int spaces = 0;
    for (size_t i = 0; i < length; i++) {  // Conta il numero degli spazi nel messaggio
        if (input[i] == ' ')
            spaces++;
    }
    if (spaces < 2) {   // Verifica che il messaggio contenga come minimo due spazi
        fprintf(stderr, "Errore: devono essere presenti come minimo due spazi, in modo da identificare correttamente l'esito, il tipo e il contenuto del messaggio.\n\n\n");
        return 0;
    }

    return 1;   // Se il messaggio passa tutti i controlli, ritorna 1
}