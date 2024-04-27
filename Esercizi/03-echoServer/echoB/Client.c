#include <arpa/inet.h>
#include <ctype.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
	int simpleSocket = 0, simplePort = 0, returnStatus = 0;
	struct sockaddr_in simpleServer;

	if(3 != argc) {
		fprintf(stderr, "\nUsage: %s <server> <port>\n", argv[0]);
		exit(1);
	}

	/* create a streaming socket */
	simpleSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if(simpleSocket == -1) {
		fprintf(stderr, "\nCould not create a socket!\n");
		exit(1);
	}
	else
		fprintf(stderr, "\nSocket created!\n");

	/* retrieve the port number for connecting */
	simplePort = atoi(argv[2]);

	/* setup the address structure */
	/* use the IP address sent as an argument for the server address */
	// bzero(&simpleServer, sizeof(simpleServer)); 
	memset(&simpleServer, '\0', sizeof(simpleServer));
	simpleServer.sin_family = AF_INET;
	// inet_addr(argv[2], &simpleServer.sin_addr.s_addr);
	simpleServer.sin_addr.s_addr = inet_addr(argv[1]);
	simpleServer.sin_port = htons(simplePort);

	/* connect to the address and port with our socket */
	returnStatus = connect(simpleSocket, (struct sockaddr *)&simpleServer, sizeof(simpleServer));

	if (returnStatus == 0)
		fprintf(stderr, "Connect successful!\n\n");
	else {
		fprintf(stderr, "Could not connect to address!\n");
		close(simpleSocket);
		exit(1);
	}

	/* send a number and send-receive the messages from the server (exercise III: echo server - b) */
	
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
	}
	else
		fprintf(stderr, "Failed connection! (RS: %d)\n", returnStatus);

	close(simpleSocket);
	return 0;
}
