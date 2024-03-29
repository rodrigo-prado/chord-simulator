#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "sha1.h"

#define MAX_MSG_LENGTH 2048
/*
unsigned int hash(char *key) {
	SHA1Context sha;
	unsigned int result = 0;
	//printf("Hash value: %s\n", key);
	SHA1Reset(&sha);
	SHA1Input(&sha, (const unsigned char*)key, strlen(key));

	if ((SHA1Result(&sha)) == 0) {
		printf("fail to compute message digest!\n");
	} else {
		result = sha.Message_Digest[0] ^ sha.Message_Digest[1];
    
    	int i=2;

		for (i = 2; i < 5; i++) {
      		result = result ^ sha.Message_Digest[i];
		}
	}
  	return result;
}
*/
unsigned int get_hash(int port) {
	/*char *str;
	char *strPort;

	str = (char *) malloc(30 * sizeof(char));
	strPort = (char *) malloc(20 * sizeof(char));
	//strcat(str,"127.0.0.1:");
	strcpy(str, "127.0.0.1:");
	sprintf(strPort, "%d", port);
	strcat(str, strPort);

	return hash(str); */
	return (port % 100);
}

int main(int argc, char* argv[]) {

	char *cmd;
	char *part;
	char *token = " ";

	cmd = (char *) malloc(50*sizeof(char));

	part = (char *) malloc(50*sizeof(char));
	
	printf("Please enter node info (query [IP address] [port]):\n");
	fgets(cmd, 50, stdin);
	memset(part, 0, 50);

	part = strtok(cmd, token);

	while (strcmp(part, "query")) {
		memset(cmd, 0, 50);
		printf("Incorrect command, please input again (query [IP address] [port]):\n");
		fgets(cmd, 50, stdin);
		part = strtok(cmd, token);
	}

	part = strtok(NULL, token);
	part = strtok(NULL, token);

	int port = atoi(part);

	char *addr = "127.0.0.1";
	int sock;
	struct sockaddr_in server_addr;

	char msg[MAX_MSG_LENGTH], reply[MAX_MSG_LENGTH];
	memset(msg, 0, MAX_MSG_LENGTH);
	memset(reply, 0, MAX_MSG_LENGTH);

	if ((sock = socket(AF_INET, SOCK_STREAM/* use tcp */, 0)) < 0) {
		perror("Nao foi possivel a criacao do socket!");
		return 1;
	}

	server_addr.sin_addr.s_addr = inet_addr(addr);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);

	if (connect(sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
		perror("Falha para conectar a uma porta via socket!");
	    return 1;
	}

	char *str;
	char *strPort;

	str = (char *) malloc(30*sizeof(char));
	strPort = (char *) malloc(20*sizeof(char));

	strcat(str, addr);
	strcat(str, ":");
	sprintf(strPort, "%d", port);
	strcat(str, strPort);

	// printf("Connection to node 127.0.0.1, port %d, position 0x%X\n", port, hash(str));
	printf("Connection to node 127.0.0.1, port %d, position %d\n", port, get_hash(port));
	memcpy(msg, "query", sizeof("query"));
	
	if (send(sock, msg, sizeof(msg), 0) < 0) {
		perror("Falha no envio (send)!");
		return 1;
	}

	// recv fingerTable from port node
	while (1) {
		fflush(stdin);
		memset(msg, 0, MAX_MSG_LENGTH);
		memset(reply, 0, MAX_MSG_LENGTH);
		
		// printf("Please enter your search key (or type 'quit' to leave):\n");
		printf("Por favor, entre com o indentificador a ser inserido (ou 'sair' para deixar a aplicacao):\n");
		fgets(msg, MAX_MSG_LENGTH,stdin);

		if (!strcmp(msg, "sair\n")) break;

		msg[sizeof(msg)-1] = '\0';
		// printf("%sHash value is 0x%X\n", msg, hash(msg));
		printf("O objeto desejado possui o indentificador %s\n", msg);
		
		if ((send(sock, msg, sizeof(msg),0)) < 0) {
			perror("Send error: ");
			return 1;
		}

		if ((recv(sock, reply, MAX_MSG_LENGTH ,0)) < 0) {
			perror("Recv error: ");
			return 1;
		}

		memset(str, 0, 30);
		strcat(str, addr);
		strcat(str, ":");
		strcat(str, reply);

		// printf("Response from node 127.0.0.1, port %d, position 0x%X\nNot found\n", atoi(reply), hash(str));
		printf("Response from node 127.0.0.1, port %d, identificador %d\n", atoi(reply), get_hash(atoi(reply)));
	}

	// printf("Query Terminated\n");
	printf("Consulta finalizada!\n");

	return 0;
}
