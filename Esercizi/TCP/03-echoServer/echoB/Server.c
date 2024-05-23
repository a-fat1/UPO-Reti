#include <arpa/inet.h>
#include <ctype.h>
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

	returnStatus = listen(simpleSocket, 5);

	if(returnStatus == -1) {
		fprintf(stderr, "Cannot listen on socket!\n");
		close(simpleSocket);
		exit(1);
	}

	char buffer[256] = "";
	struct sockaddr_in clientName = { 0 };
	int simpleChildSocket;
	unsigned int clientNameLength = sizeof(clientName);
	while(1) {
		simpleChildSocket = accept(simpleSocket, (struct sockaddr *)&clientName, &clientNameLength);

		if(simpleChildSocket == -1) {
			fprintf(stderr, "Cannot accept connections!\n");
			close(simpleSocket);
			exit(1);
		}

		/* handle the new connection request */
		/* write out MESSAGE to the client */
		/* receive a number from the client */
		/* read and write back the messages the messages */
		/* close the connection (exercise III: echo server - b) */
		returnStatus = write(simpleChildSocket, MESSAGE, strlen(MESSAGE));
		returnStatus = read(simpleChildSocket, buffer, sizeof(buffer));

		if(returnStatus > 0) {
			for(int b_char = 0; buffer[b_char] != '\0'; b_char++)
				if(!isdigit(buffer[b_char]) && buffer[b_char] != '\n') {
					returnStatus = 0;
					break;
				}

			if(returnStatus != 0) {
				int repeat = atoi(buffer);
				while(repeat > 0) {
					memset(&buffer, '\0', sizeof(buffer));
					returnStatus = read(simpleChildSocket, buffer, sizeof(buffer));
					if(returnStatus > 0) {
						write(simpleChildSocket, buffer, strlen(buffer));
						repeat--;
					}
					else
						break;
				}
			}
		}

		close(simpleChildSocket);
	}

	close(simpleSocket);
	return 0;
}
