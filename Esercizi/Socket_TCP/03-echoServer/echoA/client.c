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

	/* Il client invia il messaggio e lo riceve indietro dal server n volte (esercizio III: echo server - a). */

	char buffer[256] = "";
	returnStatus = read(simpleSocket, buffer, sizeof(buffer));

	if(returnStatus > 0) {
		fprintf(stdout, "%s", buffer);
		
		do {
			memset(buffer, '\0', sizeof(buffer));
			fprintf(stdout, "\nClient - Number of messages: ");
			if(fgets(buffer, sizeof(buffer), stdin) != NULL) {
				if(buffer[strlen(buffer) - 1] != '\n') {
					while((returnStatus = getchar()) != '\n' && returnStatus != EOF);
					fprintf(stderr, "Client - Error: input too long.\n");
					returnStatus = 0;
				} else {
					buffer[strlen(buffer) - 1] = '\0';
					returnStatus = 0;

					while(buffer[returnStatus] != '\0') {
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
						if(errno == ERANGE) {
							fprintf(stderr, "Client - Error: integer overflow.\n");
							returnStatus = 0;
						} else {
							if (returnStatus == 0)
								fprintf(stderr, "Client - Error: input equals zero.\n");
							else
								varInt = returnStatus;
						}
					}
				}
			} else {
				fprintf(stderr, "Client - Error: failed reading input.\n");
				returnStatus = 0;
			}
		} while(returnStatus == 0);

		while(varInt > 0) {
			fprintf(stdout, "\nClient - ");
			if(fgets(buffer, sizeof(buffer), stdin) != NULL) {
				if(buffer[strlen(buffer) - 1] != '\n') {
					while((returnStatus = getchar()) != '\n' && returnStatus != EOF);
					fprintf(stderr, "Client - Error: input too long.\n");
				} else {
					buffer[strlen(buffer) - 1] = '\0';
					if(strlen(buffer) != 0) {
						write(simpleSocket, buffer, strlen(buffer));

						memset(&buffer, '\0', sizeof(buffer));
						returnStatus = read(simpleSocket, buffer, sizeof(buffer));

						if(returnStatus > 0) {
							fprintf(stdout, "Server - %s\n", buffer);
							varInt--;
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
	} else
		fprintf(stderr, "Error: connection failed. (%ld)\n", returnStatus);

	close(simpleSocket);
	return 0;
}
