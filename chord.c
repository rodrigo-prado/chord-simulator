#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>
// #include <pthread.h>

#include "sha1.h"
//#include "chord.h"

// #define MSG_MAX_SIZE 		256
#define BUFFER_MAX_LENGTH 	256

/* --- Program - definition - begin */

int port = -1;
int node_id = -1;
char succ_addr[15+1] = "";  // 127.000.000.001
int succ_port = -1;

int succ_id = -1;
int pred_id = -1;

/* --- Program - definition - end */

void join_node(char *remote_addr, int remote_port, int id) {
	char msg[BUFFER_MAX_LENGTH], reply[BUFFER_MAX_LENGTH];

	memset(msg, 0, sizeof(msg));
	memset(reply, 0, sizeof(msg));

	sprintf(msg, "join %d", id);

	int sock;

	struct sockaddr_in server_addr;

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Create socket error: ");
		return;
	}

	server_addr.sin_addr.s_addr = inet_addr(remote_addr);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(remote_port);

	if(connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
		perror("Connect error in joinNode: ");
	    return;
	}

	if (send(sock, msg, sizeof(msg), 0) < 0) {
		perror("Send error: ");
		return;
	}

	//fflush(stdin);
    /*if ((read(sock, reply, MAX_MSG_LENGTH)) < 0) {
    	perror("Recv error: ");
	   	return;
	}
	*/
    //suc->port = atoi(reply);
	//suc->key = get_hash(suc->port);

	close(sock);
}

int main(int argc, char *argv[]) {
	int sockfd;
	int fd;

	socklen_t sockLen;

	struct sockaddr_in my_addr;
	struct sockaddr_in client_addr;

	if (argc == 3) { // Create network
		printf("Criando rede!\n");
		port = atoi(argv[1]);
		node_id = atoi(argv[2]);

		succ_id = node_id;
		pred_id = node_id;

		printf("Porta: %d\n", port);
		printf("ID do no: %d\n", node_id);
	} else if (argc == 5) { // Join network
		printf("Juntando a uma rede existente!\n");
		port = atoi(argv[1]);
		node_id = atoi(argv[2]);
		// remote_addr = argv[3];
		snprintf(succ_addr, 16, "%s", argv[3]);
		succ_port = atoi(argv[4]);

		printf("Porta: %d\n", port);
		printf("ID do no: %d\n", node_id);
		printf("IP remoto: %s\n", succ_addr);
		printf("Porta remota: %d\n", succ_port);

		join_node(succ_addr, succ_port, node_id);
	} else {
		printf("Boo!");
	}

	sockLen = sizeof(client_addr);

	memset((char *) &my_addr, 0, sizeof(my_addr));

	my_addr.sin_family = AF_INET;
	my_addr.sin_addr.s_addr = INADDR_ANY;
	my_addr.sin_port = htons(port);

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("cannot create socket: ");
		return 1;
	}

	if ((bind(sockfd, (struct sockaddr *) &my_addr, sizeof(client_addr))) < 0) {
		perror("bind failed: ");
		return 1;
	}

	if (listen(sockfd, 5) < 0) {
		perror("listen failed: ");
		return 1;
	}

	while (1) {
		char buffer[BUFFER_MAX_LENGTH + 1];
		// char reply[BUFFER_MAX_LENGTH + 1];

		/*
		int result;
		char *cmd;
		char *token = " ";
		*/

		char *msg_part;
		char *token = " ";

		printf("Aguardando conexao via sockets.\n");
		if ((fd = accept(sockfd, (struct sockaddr *) &client_addr, &sockLen)) < 0) {
			perror("accept failed: ");
			continue;
		}

		memset(buffer, 0, BUFFER_MAX_LENGTH + 1);

		// int len = read(fd, buffer, MSG_MAX_SIZE);
		if ((recv(fd, buffer, BUFFER_MAX_LENGTH, 0)) < 0) {
			perror("Recv error: ");
			return 1;
		}

		msg_part = strtok(buffer, token);
		if (msg_part == NULL) continue;

		if (!strcmp(msg_part, "join")) {
			msg_part = strtok(NULL, token);
			if (msg_part == NULL) continue;

			printf("%s\n", msg_part);
			printf("Anexando novo no a rede!\n");
			printf("IP address is: %s\n", inet_ntoa(client_addr.sin_addr));
			printf("port is: %d\n", (int) ntohs(client_addr.sin_port));
			//printf("Endereco IP do Cliente: %s\n", str);
			// clientPort = ntohs(client_addr);
			// printf (“Client’s port: %d”, clientPort);
			// printf("%s", inet_ntoa(client_addr.sin_addr));
		}
		// cmd = (char *) malloc(30*sizeof(char));


		// int *pSock;
		
		// pSock = (int *) malloc(sizeof(int));
		// *pSock = rqst;
		
		// pthread_create(&pthreadID, NULL, chordNode, (void *) pSock);
		// pthread_join(pthreadID, &self);
	}

}
