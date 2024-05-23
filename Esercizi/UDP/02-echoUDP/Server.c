#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
	int simpleSocket, simplePort, returnStatus;
	struct sockaddr_in simpleServer;

	if(2 != argc) {
		fprintf(stderr, "Usage: %s <port>\n", argv[0]);
		exit(1);
	}

	simpleSocket = socket(AF_INET, SOCK_DGRAM, 0);

	if(simpleSocket == -1) {
		fprintf(stderr, "Could not create a socket!\n");
		exit(1);
	}
	else
		fprintf(stderr, "\nSocket created!\n");

	simplePort = atoi(argv[1]);

	memset(&simpleServer, '\0', sizeof(simpleServer)); 
	simpleServer.sin_family = AF_INET;
	simpleServer.sin_addr.s_addr = htonl(INADDR_ANY);
	simpleServer.sin_port = htons(simplePort);

	returnStatus = bind(simpleSocket, (struct sockaddr *)&simpleServer, sizeof(simpleServer));

	if(returnStatus == 0)
		fprintf(stderr, "Bind completed!\n");
	else {
		fprintf(stderr, "Could not bind to address!\n");
		close(simpleSocket);
		exit(1);
	}
	
	char buffer[256], whoareyou[32], *pin;
	struct sockaddr_in clientName = { 0 };
	unsigned int clientNameLength = sizeof(clientName);
	while(1) {
		memset(buffer, '\0', sizeof(buffer));
		returnStatus = recvfrom(simpleSocket, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&clientName, &clientNameLength);

		if (returnStatus > 0) {
			pin = strrchr(buffer, '\n');
			if(pin != NULL) *pin = '\0';

			inet_ntop(AF_INET, &(clientName.sin_addr.s_addr), whoareyou, sizeof(whoareyou));
			printf("\n%s from %s\n", buffer, whoareyou);

			returnStatus = sendto(simpleSocket, buffer, strlen(buffer), 0, (struct sockaddr *)&clientName, clientNameLength);
			if (returnStatus > 0)
				fprintf(stderr, "Sent %s (%d bytes) back to %s\n", buffer, returnStatus, whoareyou);
			else
				fprintf(stderr, "Return Status = %d Error %d (%s)\n", returnStatus, errno, strerror(errno));
		} else
			fprintf(stderr, "Return Status = %d Error %d (%s)\n", returnStatus, errno, strerror(errno));
	}

	close(simpleSocket);
	return 0;
}
