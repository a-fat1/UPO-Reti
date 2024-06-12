#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
	int simpleSocket, simplePort;
	long returnStatus, varInt;
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

	if(returnStatus == 0)
		fprintf(stdout, "\nEcho server, write the number of messages and the messages themselves.\n\n");
	else {
		fprintf(stderr, "\nError: could not connect to address.\n");
		close(simpleSocket);
		exit(1);
	}

	/* Il client invia un numero, invia-riceve i messaggi, e infine invia 'bye' e riceve 'ack' (esercizio III: echo server - c). */

	char buffer[256] = "";
	returnStatus = read(simpleSocket, buffer, sizeof(buffer));

	if(returnStatus > 0) {
		fprintf(stdout, "%s", buffer);

		do {
			fprintf(stdout, "\nClient - Number of messages: ");
			if(fgets(buffer, sizeof(buffer), stdin) != NULL) {		// La funzione fgets() legge fino a sizeof(buffer) - 1 caratteri, o fino a '\n' o EOF.
				if(buffer[strlen(buffer) - 1] != '\n') {	// Se l'ultimo carattere non è '\n' vuol dire che sono stati inseriti troppi caratteri.
					while((returnStatus = getchar()) != '\n' && returnStatus != EOF);
					fprintf(stderr, "Client - Error: input too long.\n");
					returnStatus = 0;
				} else {
					buffer[strlen(buffer) - 1] = '\0';	// Sostituisce '\n' con '\0'.
					returnStatus = 0;

					while(buffer[returnStatus] != '\0') {	// Controlla che tutti i caratteri siano dei numeri interi.
						if(!isdigit(buffer[returnStatus])) {
							returnStatus = 0;
							break;
						} else
							returnStatus++;
					}
					
					if(returnStatus == 0)
						fprintf(stderr, "Client - Error: invalid input.\n");
					else {
						errno = 0;
						returnStatus = strtol(buffer, NULL, 10);
						if(errno == ERANGE) {	// Controllo per verificare se il numero sia maggiore di zero e che non vada in overflow.
							fprintf(stderr, "Client - Error: integer overflow.\n");
							returnStatus = 0;
						} else {
							if(returnStatus == 0)
								fprintf(stderr, "Client - Error: input equals zero.\n");
							else {
								write(simpleSocket, buffer, strlen(buffer));
								varInt = returnStatus;
							}
						}
					}
				}
			} else {
				fprintf(stderr, "Client - Error: failed reading input.\n");
				returnStatus = 0;
			}
		} while(returnStatus == 0);

		while(varInt > 0) {		// Viene utilizzato il numero precedente in input per inviare n messaggi.
			fprintf(stdout, "\nClient - ");
			if(fgets(buffer, sizeof(buffer), stdin) != NULL) {
				if(buffer[strlen(buffer) - 1] != '\n') {
					while((returnStatus = getchar()) != '\n' && returnStatus != EOF);
					fprintf(stderr, "Client - Error: invalid message.\n");
				} else {
					buffer[strlen(buffer) - 1] = '\0';
					if(strlen(buffer) != 0) {	// Controlla che il messaggio non sia vuoto, perché se si preme solo invio '\n' viene sostituito con '\0'.
						write(simpleSocket, buffer, strlen(buffer));

						memset(&buffer, '\0', sizeof(buffer));
						returnStatus = read(simpleSocket, buffer, sizeof(buffer));

						if(returnStatus > 0) {
							fprintf(stdout, "Server - %s\n", buffer);	// Stampa il messaggio ricevuto dal server.
							varInt--;	// Decrementa il numero di messaggi da inviare.
						} else {
							fprintf(stderr, "Error: connection failed. (%ld)\n", returnStatus);
							break;
						}
					} else
						fprintf(stderr, "Client - Error: invalid message.\n");
				}
			} else
				fprintf(stderr, "Client - Error: failed reading input.\n");
		}

		write(simpleSocket, "bye", strlen("bye"));	// Il client invia 'bye' al server.
		memset(&buffer, '\0', sizeof(buffer));
		returnStatus = read(simpleSocket, buffer, sizeof(buffer));	// Si mette in attesa di 'ack' dal server.
		
		if(returnStatus > 0 && strcmp(buffer, "ack") == 0)	// In ogni caso la connessione viene chiusa, anche se 'ack' non è stato ricevuto.
			fprintf(stdout, "\nAck received. Connection with server closed.\n");
	} else
		fprintf(stderr, "Error: connection failed. (%ld)\n", returnStatus);

	close(simpleSocket);
	return 0;
}
