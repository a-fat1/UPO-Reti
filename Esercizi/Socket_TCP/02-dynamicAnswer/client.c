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
		fprintf(stdout, "\nPrint message, a simple program that receives\nand displays a message from the server.\n\n");
	else {
		fprintf(stderr, "\nError: could not connect to address.\n");
		close(simpleSocket);
		exit(1);
	}

	char buffer[256] = "";
	returnStatus = read(simpleSocket, buffer, sizeof(buffer));

	/* Il client riceve e stampa a terminale la data e l'ora ricevuta dal server (esercizio II: dynamic answer). */

	if(returnStatus > 0)
		// Effettuo controllo per verificare la presenza del terminatore '\0' nella stringa ricevuta.
		fprintf(stdout, "(%d): %s", returnStatus, buffer);
	else
		fprintf(stderr, "Error: connection failed. (%d)\n", returnStatus);

	close(simpleSocket);
	return 0;
}
