#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static char MESSAGE[] = "A Message for You";

int main(int argc, char *argv[]) {
	int simpleSocket, simplePort, returnStatus;
	struct sockaddr_in simpleServer;

	if(3 != argc) {
		fprintf(stderr, "Usage: %s <server> <port>\n", argv[0]);
		exit(1);
	}

	/* create a streaming socket */
	simpleSocket = socket(AF_INET, SOCK_DGRAM, 0);

	if(simpleSocket == -1) {
		fprintf(stderr, "Could not create a socket!\n");
		exit(1);
	}
	else
		fprintf(stderr, "\nSocket created!\n");

	/* retrieve the port number for connecting */
	simplePort = atoi(argv[2]);

	/* setup the address structure */
	/* use the IP address sent as an argument for the server address  */
	// bzero(&simpleServer, sizeof(simpleServer)); 
	memset(&simpleServer, '\0', sizeof(simpleServer));
	simpleServer.sin_family = AF_INET;
	// inet_addr(argv[2], &simpleServer.sin_addr.s_addr);
	simpleServer.sin_addr.s_addr = inet_addr(argv[1]);
	simpleServer.sin_port = htons(simplePort);

	/* get the message from the server */
	returnStatus = sendto(simpleSocket, MESSAGE, sizeof(MESSAGE), 0, (struct sockaddr *)&simpleServer, sizeof(simpleServer));

	if(returnStatus < 0)
		fprintf(stderr, "Return Status = %d \n", returnStatus);

	close(simpleSocket);
	return 0;
}
