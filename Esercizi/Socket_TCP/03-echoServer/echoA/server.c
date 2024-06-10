#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

const char MESSAGE[] = "Hello UPO student!\n";

int main(int argc, char *argv[]) {
	int simpleSocket, simplePort, returnStatus;
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

	char buffer[256];
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

		/* Il server legge e restituisce i messaggi ricevuti da un client (esercizio III: echo server - a). */

		returnStatus = write(simpleChildSocket, MESSAGE, strlen(MESSAGE));
		fprintf(stdout, "\nSent welcome message.\n");

		while(1) {
			memset(&buffer, '\0', sizeof(buffer));
			fprintf(stdout, "\nWaiting for client message...\n");
			returnStatus = read(simpleChildSocket, buffer, sizeof(buffer));
		 	if(returnStatus > 0) {
				if(buffer[strlen(buffer)] != '\0' || strlen(buffer) == 0) {
					write(simpleChildSocket, "Error: invalid message.", strlen("Error: invalid message."));
					fprintf(stderr, "\nError: invalid message.\nSent error message.\n\n");
				} else {
					fprintf(stdout, "\nString from client: %s\n", buffer);
					write(simpleChildSocket, buffer, strlen(buffer));
					fprintf(stdout, "Sent string '%s'.\n", buffer);
				}
			} else {
				fprintf(stderr, "\nError: connection failed. (%d)\n", returnStatus);
				break;
			}
		}

		close(simpleChildSocket);
		fprintf(stdout, "Connection closed.\n");
	}

	close(simpleSocket);
	return 0;
}
