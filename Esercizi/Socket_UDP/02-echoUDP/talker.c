#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
	int simpleSocket, simplePort, returnStatus;
	struct sockaddr_in simpleServer;

	if(3 != argc) {
		fprintf(stderr, "Usage: %s <server> <port>\n", argv[0]);
		exit(1);
	}

	simpleSocket = socket(AF_INET, SOCK_DGRAM, 0);

	if(simpleSocket == -1) {
		fprintf(stderr, "\nError: could not create a socket.\n");
		exit(1);
	} else
		fprintf(stdout, "\nEcho client, type 'bye' to exit.\n");

	simplePort = atoi(argv[2]);

	memset(&simpleServer, '\0', sizeof(simpleServer));
	simpleServer.sin_family = AF_INET;
	simpleServer.sin_addr.s_addr = inet_addr(argv[1]);
	simpleServer.sin_port = htons(simplePort);

	/* Il client continua ad inviare-ricevere i messaggi fino a quando non invia 'bye' al server (esercizio I: echo server - UDP). */

	char buffer[256];
	struct sockaddr_in senderAddr;
	socklen_t len = sizeof(senderAddr);
	while(1) {
		fprintf(stdout, "\nClient - ");
		if(fgets(buffer, sizeof(buffer), stdin) != NULL) {		// La funzione fgets() legge fino a sizeof(buffer) - 1 caratteri, o fino a '\n' o EOF.
			if(buffer[strlen(buffer) - 1] != '\n') {	// Se l'ultimo carattere non è '\n' vuol dire che sono stati inseriti troppi caratteri.
				while((returnStatus = getchar()) != '\n' && returnStatus != EOF);
				fprintf(stderr, "Client - Error: input too long.\n");
			} else {
				buffer[strlen(buffer) - 1] = '\0';	// Sostituisce '\n' con '\0' e invia il messaggio al server.
				returnStatus = sendto(simpleSocket, buffer, sizeof(buffer), 0, (struct sockaddr *)&simpleServer, sizeof(simpleServer));

				if(returnStatus > 0) {
					memset(buffer, '\0', sizeof(buffer));	// Il client ripulisce il buffer e riceve la risposta del server.
					returnStatus = recvfrom(simpleSocket, buffer, sizeof(buffer), 0, (struct sockaddr *)&senderAddr, &len);
					if(returnStatus > 0) {
						fprintf(stdout, "Server - %s\n", buffer);	// Il client stampa la risposta del server, e con la parola 'bye' termina la connessione.
						if(strcmp(buffer, "bye") == 0)
							break;
					} else {
						fprintf(stderr, "Return Status = %d\n", returnStatus);
						break;
					}
				} else {
					fprintf(stderr, "Return Status = %d\n", returnStatus);
					break;
				}
			}
		} else
			fprintf(stderr, "Client - Error reading input.\n");
	}

	close(simpleSocket);
	return 0;
}
