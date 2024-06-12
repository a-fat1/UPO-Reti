#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
	int simpleSocket, simplePort;
	long returnStatus;
	struct sockaddr_in simpleServer;

	if(3 != argc) {
		fprintf(stderr, "\nUsage: %s <server> <port>\n", argv[0]);
		exit(1);
	}

	simpleSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if(simpleSocket == -1) {
		fprintf(stderr, "\nError: could not create a socket.\n");
		exit(1);
	}

	simplePort = atoi(argv[2]);

	memset(&simpleServer, '\0', sizeof(simpleServer));
	simpleServer.sin_family = AF_INET;
	simpleServer.sin_addr.s_addr = inet_addr(argv[1]);
	simpleServer.sin_port = htons(simplePort);

	returnStatus = connect(simpleSocket, (struct sockaddr *)&simpleServer, sizeof(simpleServer));

	if (returnStatus == 0)
		fprintf(stdout, "\nMax server, enter the largest number to store\nor type 'bye' to terminate.\n\n");
	else {
		fprintf(stderr, "\nError: could not connect to address.\n");
		close(simpleSocket);
		exit(1);
	}

	/* Il client digita un numero e lo invia. */
	/* Riceve indietro il numero massimo inviato (esercizio IV: max server). */

	char buffer[256] = "";
	returnStatus = read(simpleSocket, buffer, sizeof(buffer));
	if(returnStatus > 0) {
		fprintf(stdout, "%s", buffer);

		do {
			fprintf(stdout, "\nClient - Type an integer number: ");
			if(fgets(buffer, sizeof(buffer), stdin) != NULL) {		// La funzione fgets() legge fino a sizeof(buffer) - 1 caratteri, o fino a '\n' o EOF.
				if(buffer[strlen(buffer) - 1] != '\n') {	// Se l'ultimo carattere non è '\n' vuol dire che sono stati inseriti troppi caratteri.
					while((returnStatus = getchar()) != '\n' && returnStatus != EOF);
					fprintf(stderr, "Client - Error: input too long.\n");
				} else {
					buffer[strlen(buffer) - 1] = '\0';	// Sostituisce '\n' con '\0'.

					if(buffer[0] == '-')	// Se il numero è negativo, i controlli partono dal secondo carattere.
						returnStatus = 1;
					else
						returnStatus = 0;

					if(strcmp(buffer, "bye") != 0) {
						while(buffer[returnStatus] != '\0') {	// Controlla che tutti i caratteri siano dei numeri interi.
							if(!isdigit(buffer[returnStatus])) {
								returnStatus = 0;
								break;
							} else
								returnStatus++;
						}

						if(returnStatus == 0) {
							fprintf(stderr, "Client - Error: invalid input.\n");
							returnStatus = -1;
						} else {
							errno = 0;
    						returnStatus = strtol(buffer, NULL, 10);
							if(errno == ERANGE) {
								if(returnStatus == LONG_MAX)	// Il client controlla se il numero contenuto nella stringa non generi overflow.
									fprintf(stderr, "Client - Error: integer overflow.\n");
								else if (returnStatus == LONG_MIN)	// Lo stesso discorso si applica per l'underflow.
									fprintf(stderr, "Client - Error: integer underflow.\n");
								returnStatus = -1;
							} else
								returnStatus = 1;
						}
					} else
						returnStatus = 1;

					if(returnStatus == 1) {
						write(simpleSocket, buffer, strlen(buffer));	// Il client invia il numero al server, oppure invia la parola 'bye'.
						memset(buffer, '\0', sizeof(buffer));

						returnStatus = read(simpleSocket, buffer, sizeof(buffer));	// Il client riceve la risposta dal server.
						if(returnStatus > 0) {
							if(strcmp(buffer, "ack") == 0) {	// La parola 'ack' viene ricevuta dopo 'bye', e successivamente la connessione viene terminata.
								fprintf(stdout, "Client - Ack received, goodbye.\n");
								returnStatus = 0;
							} else
								fprintf(stdout, "Server - %s\n", buffer);	// Altrimenti il client stampa a schermo il numero massimo che il server ha memorizzato.
						} else {
							fprintf(stderr, "Client - Error receiving data from server.\n");
							returnStatus = 0;
						}
					}
				}
			} else
				fprintf(stderr, "Client - Error reading input.\n");

		} while(returnStatus != 0);

	} else
		fprintf(stderr, "Client - Error receiving data from server.\n");

	close(simpleSocket);
	return 0;
}
