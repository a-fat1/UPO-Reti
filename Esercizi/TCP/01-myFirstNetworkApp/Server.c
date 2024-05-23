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
		fprintf(stderr, "\nCould not create a socket!\n");
		exit(1);
	}
	else
		fprintf(stderr, "\nSocket created!\n");

	/* retrieve the port number for listening */
	simplePort = atoi(argv[1]);

	/* setup the address structure */
	/* use INADDR_ANY to bind to all local addresses */
	memset(&simpleServer, '\0', sizeof(simpleServer)); 
	simpleServer.sin_family = AF_INET;
	simpleServer.sin_addr.s_addr = htonl(INADDR_ANY);
	simpleServer.sin_port = htons(simplePort);

	/* bind to the address and port with our socket */
	returnStatus = bind(simpleSocket, (struct sockaddr *)&simpleServer, sizeof(simpleServer));

	if(returnStatus == 0)
		fprintf(stderr, "Bind completed!\n");
	else {
		fprintf(stderr, "Could not bind to address!\n");
		close(simpleSocket);
		exit(1);
	}

	/* lets listen on the socket for connections */
	returnStatus = listen(simpleSocket, 5);

	if(returnStatus == -1) {
		fprintf(stderr, "Cannot listen on socket!\n");
		close(simpleSocket);
		exit(1);
	}

	struct sockaddr_in clientName = { 0 };
	int simpleChildSocket;
	unsigned int clientNameLength = sizeof(clientName);
	while(1) {
		/* wait here */
		simpleChildSocket = accept(simpleSocket, (struct sockaddr *)&clientName, &clientNameLength);

		if(simpleChildSocket == -1) {
			fprintf(stderr, "Cannot accept connections!\n");
			close(simpleSocket);
			exit(1);
		}

		/* handle the new connection request */
		/* write out our message to the client */
		write(simpleChildSocket, MESSAGE, strlen(MESSAGE));
		close(simpleChildSocket);
	}

	close(simpleSocket);
	return 0;
}
