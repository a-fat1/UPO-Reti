#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

const char MESSAGE[] = "Hello UPO student!\n";

int main(int argc, char *argv[]) {
	int simpleSocket, simplePort;
	long returnStatus, clientNumber;
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

	int simpleChildSocket, firstNumber;
	char buffer[256];
	struct sockaddr_in clientName = { 0 };
	unsigned int clientNameLength = sizeof(clientName);
	while(1) {
		fprintf(stdout, "\nWaiting for a connection...\n");
		simpleChildSocket = accept(simpleSocket, (struct sockaddr *)&clientName, &clientNameLength);

		if(simpleChildSocket == -1) {
			fprintf(stderr, "Error: cannot accept connections!\n");
			close(simpleSocket);
			exit(1);
		} else
			fprintf(stdout, "Connection established.\n");

		/* Il server riceve un numero dal client. */
		/* Restituisce il numero massimo ricevuto (esercizio IV: max server). */

		write(simpleChildSocket, MESSAGE, strlen(MESSAGE));
		firstNumber = 1;

		do {
			memset(buffer, '\0', sizeof(buffer));
			fprintf(stdout, "\nWaiting for client number...\n");
			returnStatus = read(simpleChildSocket, buffer, sizeof(buffer));

			if(returnStatus > 0) {
				if(buffer[strlen(buffer)] != '\0') {
					write(simpleChildSocket, "Error: invalid number.", strlen("Error: invalid number."));
					fprintf(stderr, "\nError: invalid number.\nSent error number.\n\n");
					returnStatus = 0;
				} else {
					fprintf(stdout, "\nString from client: %s\n", buffer);
					if(strcmp(buffer, "bye") != 0) {
						if(buffer[0] == '-' && strlen(buffer) > 1)
							returnStatus = 1;
						else
							returnStatus = 0;

						while(buffer[returnStatus] != '\0') {
							if(!isdigit(buffer[returnStatus])) {
								returnStatus = 0;
								break;
							} else
								returnStatus++;
						}

						if(returnStatus != 0) {
							errno = 0;
							returnStatus = strtol(buffer, NULL, 10);
							if(errno == ERANGE) {
								if(returnStatus == LONG_MAX) {
									snprintf(buffer, sizeof(buffer), "Error: integer overflow.\n");
									fprintf(stderr, "Error: integer overflow.\n");
								} else if (returnStatus == LONG_MIN) {
									snprintf(buffer, sizeof(buffer), "Error: integer underflow.\n");
									fprintf(stderr, "Error: integer underflow.\n");
								}
							} else {
								if(!firstNumber) {
									if(returnStatus > clientNumber)
										clientNumber = returnStatus;
								} else {
									clientNumber = returnStatus;
									firstNumber = 0;
								}
								snprintf(buffer, sizeof(buffer), "Max number saved: %ld", clientNumber);
								fprintf(stdout, "Number saved: %ld\n", clientNumber);
							}
							
							write(simpleChildSocket, buffer, strlen(buffer));
							returnStatus = 1;
						} else {
							write(simpleChildSocket, "Error: invalid input.", strlen("Error: invalid input."));
							fprintf(stderr, "Error: invalid input.\n");
						}
					} else {	// In caso del messaggio "bye" il server invia "ack" al client
						fprintf(stdout, "\nReceived 'bye'.\n");
						write(simpleChildSocket, "ack", strlen("ack"));
						fprintf(stdout, "Sent 'ack'.\n");
					}
				}
			} else
				fprintf(stderr, "Connection failed. (%ld)\n", returnStatus);
		} while((strcmp(buffer, "bye") != 0 && returnStatus > 0));

		close(simpleChildSocket);
		fprintf(stdout, "\nConnection closed.\n");
	}

	close(simpleSocket);
	return 0;
}
