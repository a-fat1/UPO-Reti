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
		fprintf(stderr, "Could not create a socket!\n");
		exit(1);
	}
	else
		fprintf(stderr, "\nSocket created!\n\nEcho client, type 'bye' to exit\n");

	simplePort = atoi(argv[2]);

	memset(&simpleServer, '\0', sizeof(simpleServer));
	simpleServer.sin_family = AF_INET;
	simpleServer.sin_addr.s_addr = inet_addr(argv[1]);
	simpleServer.sin_port = htons(simplePort);

	char buffer[256];
	struct sockaddr_in senderAddr;
	socklen_t len = sizeof(senderAddr);
	while(1) {
		fprintf(stdout, "\nMessage: ");
		fgets(buffer, sizeof(buffer), stdin);

		returnStatus = sendto(simpleSocket, buffer, sizeof(buffer), 0, (struct sockaddr *)&simpleServer, sizeof(simpleServer));

		if(returnStatus > 0) {
			memset(buffer, '\0', sizeof(buffer));
			returnStatus = recvfrom(simpleSocket, buffer, sizeof(buffer), 0, (struct sockaddr *)&senderAddr, &len);
			if(returnStatus > 0) {
				printf("Echo: %s\n", buffer);
				if(strcmp(buffer, "bye") == 0)
					break;
			}
			else {
				fprintf(stderr, "Return Status = %d \n", returnStatus);
				break;
			}
		}
		else {
			fprintf(stderr, "Return Status = %d \n", returnStatus);
			break;
		}
	}

	close(simpleSocket);
	return 0;
}
