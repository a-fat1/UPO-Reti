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

	/* type a number, send a number, receive back the max number sent (exercise IV: max server) */
	char buffer[256] = "";
	returnStatus = read(simpleSocket, buffer, sizeof(buffer));
	fprintf(stdout, "%s", buffer);

	do {
		fprintf(stdout, "\nType an integer number\nor type bye to end the programm: ");
		fgets(buffer, sizeof(buffer), stdin);

		returnStatus = 1;
		if(strcmp(buffer, "bye\n") != 0) {
			for(int bChar = 0; buffer[bChar] != '\0'; bChar++)
				if(!isdigit(buffer[bChar]) && buffer[bChar] != '\n') {
					returnStatus = 0;
					break;
				}
		}

		if(returnStatus == 0) {
			fprintf(stdout, "Invalid input.\n");
			returnStatus = 1;
		}
		else {
			write(simpleSocket, buffer, sizeof(buffer));
			memset(buffer, '\0', sizeof(buffer));
			returnStatus = read(simpleSocket, buffer, sizeof(buffer));
			if(strcmp(buffer, "ack") == 0 || returnStatus == 0) {
				fprintf(stdout, "Ack received. Goodbye.\n");
				returnStatus = 0;
			}
			else
				fprintf(stdout, "%s\n", buffer);
		}
	} while(returnStatus != 0);

	close(simpleSocket);
	return 0;
}
