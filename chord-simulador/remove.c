#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAX_MSG_LENGTH 2048

unsigned int get_hash(int port) {
	return (port % 100);
}

int main(int argc, char* argv[]) {

	char *cmd;
	char *part;
	char *token = " ";

	cmd = (char *) malloc(50*sizeof(char));

	part = (char *) malloc(50*sizeof(char));
	
	//printf("Please enter node info (query [IP address] [port]):\n");
	printf("Por favor, entre com as informacoes de endereco IP e Porta \n(remove-no [IP address] [port]):\n");
	fgets(cmd, 50, stdin);
	memset(part, 0, 50);

	part = strtok(cmd, token);

	while (strcmp(part, "remove-no")) {
		memset(cmd, 0, 50);
		// printf("Incorrect command, please input again (query [IP address] [port]):\n");
		printf("Comando incorreto! \nPor favor, entre com as informacoes de endereco IP e Porta \n(insere-no [IP address] [port]):\n");
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
	printf("Conectando ao no 127.0.0.1, porta %d, identificador %d\n", port, get_hash(port));
	memcpy(msg, "remove-no", sizeof("remove-no"));
	
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
		printf("Por favor, entre com o indentificador a ser removido (ou 'sair' para deixar a aplicacao):\n");
		fgets(msg, MAX_MSG_LENGTH,stdin);

		if (!strcmp(msg, "sair\n")) break;

		msg[sizeof(msg)-1] = '\0';
		// printf("%sHash value is 0x%X\n", msg, hash(msg));
		printf("O objeto que se deseja remover possui o indentificador %s\n", msg);
		
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
	printf("Remocoes finalizadas!\n");

	return 0;
}
