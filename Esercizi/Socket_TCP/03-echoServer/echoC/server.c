#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

const char MESSAGE[] = "Hello UPO student!\n";

int main(int argc, char *argv[]) {
	int simpleSocket, simplePort;
	long returnStatus, repeatMessage;
	struct sockaddr_in simpleServer;

	if(2 != argc) {
		fprintf(stderr, "\nUsage: %s <port>\n", argv[0]);
		exit(1);
	}

	simpleSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if(simpleSocket == -1) {
		fprintf(stderr, "\nError: could not create a socket.\n");
		exit(1);
	} else
		fprintf(stdout, "\nSocket created.\n");

	simplePort = atoi(argv[1]);

	memset(&simpleServer, '\0', sizeof(simpleServer)); 
	simpleServer.sin_family = AF_INET;
	simpleServer.sin_addr.s_addr = htonl(INADDR_ANY);
	simpleServer.sin_port = htons(simplePort);

	returnStatus = bind(simpleSocket, (struct sockaddr *)&simpleServer, sizeof(simpleServer));

	if(returnStatus == 0)
		fprintf(stdout, "Bind completed.\n");
	else {
		fprintf(stderr, "Error: could not bind to address.\n");
		close(simpleSocket);
		exit(1);
	}

	returnStatus = listen(simpleSocket, 5);

	if(returnStatus == -1) {
		fprintf(stderr, "Error: cannot listen on socket.\n");
		close(simpleSocket);
		exit(1);
	}

	char buffer[256] = "";
	struct sockaddr_in clientName = { 0 };
	int simpleChildSocket;
	unsigned int clientNameLength = sizeof(clientName);
	while(1) {
		fprintf(stdout, "\nWaiting for a connection...\n");
		simpleChildSocket = accept(simpleSocket, (struct sockaddr *)&clientName, &clientNameLength);

		if(simpleChildSocket == -1) {
			fprintf(stderr, "Error: cannot accept connections.\n");
			close(simpleSocket);
			exit(1);
		} else
			fprintf(stdout, "Connection established.\n");

		/* Il server invia il messaggio di benvenuto al client. */
		/* Riceve un numero dal client. */
		/* Riceve e rispedisce indietro i messaggi. */
		/* Riceve 'bye', invia 'ack' e chiude la connessione (esercizio III: echo server - c). */

		returnStatus = write(simpleChildSocket, MESSAGE, strlen(MESSAGE));

		memset(&buffer, '\0', sizeof(buffer));
		fprintf(stdout, "\nWaiting for number...\n");
		returnStatus = read(simpleChildSocket, buffer, sizeof(buffer));		// Il server riceve un numero dal client.

		if(returnStatus > 0) {
			if(buffer[strlen(buffer)] != '\0') {	// Il server controlla se il numero contenuto nella stringa abbia il terminatore '\0'.
				write(simpleChildSocket, "Error: invalid number.", strlen("Error: invalid number."));
				fprintf(stderr, "\nError: invalid number.\nSent error number.\n\n");
			} else {
				returnStatus = 0;
				while(buffer[returnStatus] != '\0') {	// Il server controlla se il numero contenuto nella stringa sia un intero.
					if(!isdigit(buffer[returnStatus])) {
						returnStatus = 0;
						break;
					} else
						returnStatus++;
				}

				if(returnStatus != 0) {
					errno = 0;
					returnStatus = strtol(buffer, NULL, 10);
					if(errno == ERANGE) {	// Il server controlla se il numero contenuto nella stringa non generi overflow.
						fprintf(stderr, "Error: integer overflow.\n");
						returnStatus = 0;
					} else {
						if (returnStatus == 0)	// Viene anche effettuato il controllo per verificare che numero > 0.
							fprintf(stderr, "Error: input equals zero.\n");
						else {
							repeatMessage = returnStatus;	// Superati tutti i controlli, il server salva il numero di messaggi da ricevere.
							fprintf(stdout, "Number from client: %ld.\n", repeatMessage);
						}
					}
				}

				if(returnStatus != 0) {
					while(repeatMessage > 0) {	// Il server riceve e rispedisce indietro il numero di messaggi prestabiliti.
						memset(&buffer, '\0', sizeof(buffer));
						fprintf(stdout, "\nWaiting for client message...\n");
						returnStatus = read(simpleChildSocket, buffer, sizeof(buffer));
						if(returnStatus > 0) {
							if(buffer[strlen(buffer)] != '\0' || strlen(buffer) == 0) {		// Il server controlla se il messaggio ricevuto ha il terminatore '\0' e non è nullo.
								write(simpleChildSocket, "Error: invalid message.", strlen("Error: invalid message."));
								fprintf(stderr, "\nError: invalid message.\nSent error message.\n\n");
							} else {
								fprintf(stdout, "\nString from client: '%s'.\n", buffer);
								write(simpleChildSocket, buffer, strlen(buffer));
								fprintf(stdout, "Sent string '%s'.\n", buffer);
							}

							repeatMessage--;
						} else {
							fprintf(stderr, "\nError: connection failed. (%ld)\n", returnStatus);
							break;
						}
					}
					memset(&buffer, '\0', sizeof(buffer));	// Il server pulisce il buffer per ricevere 'bye' dal client.

					returnStatus = read(simpleChildSocket, buffer, sizeof(buffer));
					if(returnStatus > 0 && strcmp(buffer, "bye") == 0) {	// In ogni caso la connessione viene chiusa se il server non riceve nulla.
						fprintf(stdout, "\nReceived 'bye'.\n");
						write(simpleChildSocket, "ack", strlen("ack"));		// Il server invia 'ack' se riceve 'bye'.
						fprintf(stdout, "Sent 'ack'.\n");
					}
				} else
					fprintf(stderr, "\nError: connection failed. (%ld)\n", returnStatus);
			}
		} else
			fprintf(stderr, "\nError: connection failed. (%ld)\n", returnStatus);

		close(simpleChildSocket);
		fprintf(stdout, "\nConnection closed.\n");
	}

	close(simpleSocket);
	return 0;
}
