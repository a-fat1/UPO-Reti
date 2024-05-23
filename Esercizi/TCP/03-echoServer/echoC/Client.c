#include <arpa/inet.h>
#include <ctype.h>
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
		fprintf(stderr, "\nCould not create a socket!\n");
		exit(1);
	}
	else
		fprintf(stderr, "\nSocket created!\n");

	simplePort = atoi(argv[2]);

	memset(&simpleServer, '\0', sizeof(simpleServer));
	simpleServer.sin_family = AF_INET;
	simpleServer.sin_addr.s_addr = inet_addr(argv[1]);
	simpleServer.sin_port = htons(simplePort);

	returnStatus = connect(simpleSocket, (struct sockaddr *)&simpleServer, sizeof(simpleServer));

	if (returnStatus == 0)
		fprintf(stderr, "Connect successful!\n\n");
	else {
		fprintf(stderr, "Could not connect to address!\n");
		close(simpleSocket);
		exit(1);
	}

	/* send a number, send-receive the messages, send bye and receive ack (exercise III: echo server - c) */
	char buffer[256] = "";
	returnStatus = read(simpleSocket, buffer, sizeof(buffer));

	if(returnStatus > 0) {
		fprintf(stdout, "%s", buffer);

		int varInt;
		fprintf(stdout, "Type the number of messages to send to the server: ");
		do {
			fgets(buffer, sizeof(buffer), stdin);

			returnStatus = 1;
			for(varInt = 0; buffer[varInt] != '\0'; varInt++)
				if(!isdigit(buffer[varInt]) && buffer[varInt] != '\n') {
					returnStatus = 0;
					break;
				}

			if(returnStatus == 0)
				fprintf(stdout, "Invalid input.\n\nType an integer number of messages: ");
			else
				write(simpleSocket, buffer, sizeof(buffer));
		} while(returnStatus == 0);

		varInt = atoi(buffer);

		while(varInt > 0) {
			fprintf(stdout, "\nType here your message: ");
			fgets(buffer, sizeof(buffer), stdin);

			write(simpleSocket, buffer, sizeof(buffer));
			memset(&buffer, '\0', sizeof(buffer));
			returnStatus = read(simpleSocket, buffer, sizeof(buffer));

			if(returnStatus > 0) {
				fprintf(stdout, "Echo: %s", buffer);
				varInt--;
			}
			else {
				fprintf(stderr, "Failed connection! (RS: %d)\n", returnStatus);
				break;
			}
		}

		write(simpleSocket, "bye", strlen("bye"));
		memset(&buffer, '\0', sizeof(buffer));
		returnStatus = read(simpleSocket, buffer, sizeof(buffer));
		
		if(returnStatus > 0 && strcmp(buffer, "ack") == 0)
			fprintf(stdout, "\nAck received. Connection with server closed.\n");
	}
	else
		fprintf(stderr, "Failed connection! (RS: %d)\n", returnStatus);

	close(simpleSocket);
	return 0;
}
