#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>

#include "chord.h"

unsigned int get_hash(int port) {
	return (port % 100);
}

int find(unsigned int value) {

	if (value == node.key) {
		return node.port;
	} else if ((value > node.key) && (value <= suc->key)) {
		return suc->port;
	} else if (((value > node.key) || (value < suc->key))
			&& (node.key > suc->key)){
		return suc->port;
	} else {
		int i;

	    for (i = MBITS-1; i >= 0; i--) {
			if (((node.key < fingerTable[i].nodeInfo->key) &&
						(fingerTable[i].nodeInfo->key < value))
					||
					((value < node.key) &&
							(fingerTable[i].nodeInfo->key > node.key))
					||
					((value < node.key) &&
							(fingerTable[i].nodeInfo->key < value))) {
				int sock;
				struct sockaddr_in server_addr;
				char msg[MAX_MSG_LENGTH], reply[MAX_MSG_LENGTH];

				if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
					perror("Nao foi possivel a criacao do socket!");
				    return 0;
				}

				server_addr.sin_addr.s_addr = inet_addr(addr);
				server_addr.sin_family = AF_INET;
				server_addr.sin_port = htons(fingerTable[i].nodeInfo->port);

				if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
					perror("Falha para conectar a uma porta via socket!");
					return 0;
				}

				memset(msg, 0, sizeof(msg));
				sprintf(msg, "find %u", value);

				if (send(sock, msg, MAX_MSG_LENGTH, 0) < 0) {
				    perror("Falha no envio (send)!");
				    return 0;
				}

				memset(reply, 0, sizeof(reply));
				if (recv(sock, reply, MAX_MSG_LENGTH, 0) < 0) {
					perror("Recv error: ");
					return 0;
				}

				int recvPort;
				recvPort = atoi(reply);
				close(sock);
				return recvPort;
			}
	    }
    }
	return node.port;
}

void fix_fingers(int i) {
	int result;

	result = find(fingerTable[i].start);

	while (result == 0) {
		result = find(fingerTable[i].start);
	}

	fingerTable[i].nodeInfo->port = result;
	fingerTable[i].nodeInfo->key = get_hash(fingerTable[i].nodeInfo->port);
}

void notify() {
	int sock;
	struct sockaddr_in server_addr;
	char msg[MAX_MSG_LENGTH], reply[MAX_MSG_LENGTH];

	memset(reply, 0, sizeof(msg));


	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Nao foi possivel a criacao do socket!");
	    return;
	}

	server_addr.sin_addr.s_addr = inet_addr(addr);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(suc->port);

	if (connect(sock, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0) {
		suc->port = suc2->port;
		suc->key = get_hash(suc->port);
		suc2->port = node.port;
		suc2->key = get_hash(suc2->port);
		server_addr.sin_port = htons(suc->port);
		reset_pre(suc->port);

		if (connect(sock, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0) {
			perror("Falha para conectar a uma porta via socket!");
			return;
		}
	}

	memset(msg, 0, sizeof(msg));
	sprintf(msg, "notify %d", node.port);

	if (send(sock, msg, MAX_MSG_LENGTH, 0) < 0) {
	    perror("Falha no envio (send)!");
	    return;
	}

	if ((read(sock, reply, MAX_MSG_LENGTH)) < 0) {
    	perror("Recv error: ");
	   	return;
	}
	int indice = atoi(reply);
	while (indice != -1) {
		//printf("\nindice: %d\n\n", indice);
		chaves[indice] = 1;

		//fflush(stdin);
	    if ((read(sock, reply, MAX_MSG_LENGTH)) < 0) {
	    	perror("Recv error: ");
		   	return;
		}
		indice = atoi(reply);
	}

	close(sock);
}

void stabilize() {
	int sock;
	struct sockaddr_in server_addr;
	char msg[MAX_MSG_LENGTH], reply[MAX_MSG_LENGTH];

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Nao foi possivel a criacao do socket!");
	    return;
	}

	server_addr.sin_addr.s_addr = inet_addr(addr);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(suc->port);

	if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
		suc->port = suc2->port;
		suc->key = get_hash(suc->port);
		suc2->port = node.port;
		suc2->key = get_hash(suc2->port);
		server_addr.sin_port = htons(suc->port);
		reset_pre(suc->port);

		if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
			perror("Falha para conectar a uma porta via socket!");
			return;
		}
	}

	memset(msg, 0, sizeof(msg));
	sprintf(msg,"stable %d",node.port);

	if (send(sock, msg, MAX_MSG_LENGTH, 0) < 0) {
	    perror("Falha no envio (send)!");
	    return;
	}

	memset(reply, 0, sizeof(reply));
	if (recv(sock, reply, MAX_MSG_LENGTH, 0) < 0) {
		perror("Recv error: ");
		return;
	}

	int recvPort;
	unsigned int recvHash;

	recvPort = atoi(reply);
	recvHash = get_hash(recvPort);

	if (recvPort > 0) {
	    if ((suc->key > node.key) && (node.key < recvHash) && (recvHash < suc->key)) {
	    	suc->port = recvPort;
	    	suc->key = get_hash(suc->port);
	    } else if ((suc->key < node.key) && (recvPort!=node.port) &&
	    		((recvHash > node.key) || (recvHash < suc->key))) {
	    	suc->port = atoi(reply);
	    	suc->key = get_hash(suc->port);
	    }
	}
	close(sock);
}

void reset_pre(int port) {
	int sock;
	struct sockaddr_in server_addr;
	char msg[MAX_MSG_LENGTH];

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Nao foi possivel a criacao do socket!");
	    return;
	}

	server_addr.sin_addr.s_addr = inet_addr(addr);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);

	if (connect(sock, (struct sockaddr *) & server_addr, sizeof(server_addr)) < 0) {
		perror("Falha para conectar a uma porta via socket!");
		return;
	}

	memset(msg,0,sizeof(msg));
	sprintf(msg,"reset-pre");

	if (send(sock, msg, MAX_MSG_LENGTH, 0) < 0) {
	    perror("Falha no envio (send)!");
	    return;
	}
	close(sock);
}

void keep_alive() {
	if (pre!=NULL) {
		int sock;
		struct sockaddr_in server_addr;
		char msg[MAX_MSG_LENGTH];

		if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
			perror("Nao foi possivel a criacao do socket!");
		    return;
		}

		server_addr.sin_addr.s_addr = inet_addr(addr);
		server_addr.sin_family = AF_INET;
		server_addr.sin_port = htons(pre->port);

		if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
			free(pre);
			pre = NULL;
			perror("Falha para conectar a uma porta via socket!");
			return;
		}

		memset(msg, 0, sizeof(msg));
		sprintf(msg, "keep-alive %d", suc->port);

		if (send(sock, msg, MAX_MSG_LENGTH, 0) < 0) {
		    perror("Falha no envio (send)!");
		    return;
		}

		close(sock);
	}
}

void *update() {
	int i;
	while (1) {
		for (i = 0; i < MBITS; i++) {
			fix_fingers(i);
		}
		sleep(1);
		stabilize();
		notify();
		keep_alive();
	}
	pthread_exit(0);
}

void *command() {
	char *cmd;

	cmd = (char *) malloc(MAX_MSG_LENGTH*sizeof(char));
	memset(cmd, 0, MAX_MSG_LENGTH);
	while (strcmp(cmd, "remover")) {
		fscanf(stdin, "%s", cmd);
	}

	printf("\n\nSaindo do remover!\n");

	for (int i = 0; i < 128; ++i) {
		// printf("iteracao %d \n", i);
		if (chaves[i]) {

			int sock;
			struct sockaddr_in server_addr;
			char msg[MAX_MSG_LENGTH];
			//char reply[MAX_MSG_LENGTH];

			if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
				perror("Nao foi possivel a criacao do socket!");
			    return 0;
			}

			server_addr.sin_addr.s_addr = inet_addr(addr);
			server_addr.sin_family = AF_INET;
			server_addr.sin_port = htons(suc->port);

			if (connect(sock, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0) {
				perror("Falha para conectar a uma porta via socket!");
				return 0;
			}

			memset(msg, 0, sizeof(msg));
			sprintf(msg, "adiciona-no %u", i);

			if (send(sock, msg, MAX_MSG_LENGTH, 0) < 0) {
			    perror("Falha no envio (send)!");
			    return 0;
			}

			/*memset(reply, 0, sizeof(reply));
			if (recv(sock, reply, MAX_MSG_LENGTH, 0) < 0) {
				perror("Recv error: ");
				return 0;
			}*/

			// close(sock);
		}
	}
	sleep(9);
	exit(0);
}

void *chordNode(void *sock) {
	char msg[MAX_MSG_LENGTH];
	char reply[MAX_MSG_LENGTH];
	int result;
	char *cmd;

	cmd = (char *) malloc(30*sizeof(char));

	char *token = " ";
	int rqst = *(int *) sock;
	free(sock);

	while ((recv(rqst, msg, sizeof(msg), 0)) > 0) {
    	cmd = strtok(msg, token);

    	if (!strcmp(cmd, "join")) {
    		cmd = strtok(NULL, token);
			pcNode n;
			n = (pcNode) malloc(sizeof(cNode));
			n->port = atoi(cmd);
			n->key = get_hash(n->port);

			if (suc->port == node.port) {

				suc->port=n->port;
				suc->key = get_hash(suc->port);

				pre->port = n->port;
				pre->key = get_hash(pre->port);

				result = node.port;
			} else {
				result = find(n->key);
				while (result == 0) {
					result = find(n->key);
				}
			}

			memset(reply, 0, MAX_MSG_LENGTH);
			sprintf(reply, "%d", result);

			if (send(rqst, reply, MAX_MSG_LENGTH, 0) < 0) {
				perror("Falha no envio (send)!");
				return NULL;
			}

			for (int i = 0; i < 128; ++i) {
				if (chaves[i]) {
					memset(reply, 0, MAX_MSG_LENGTH);
					sprintf(reply, "%d", i);
					if (send(rqst, reply, MAX_MSG_LENGTH, 0) < 0) {
						perror("Falha no envio (send)!");
						return NULL;
					}
					chaves[i] = 0;
				}
			}

			memset(reply, 0, MAX_MSG_LENGTH);
			sprintf(reply, "%d", -1);
			if (send(rqst, reply, MAX_MSG_LENGTH, 0) < 0) {
				perror("Falha no envio (send)!");
				return NULL;
			}
			
    	} else if (!strcmp(cmd, "find")) {
    		cmd = strtok(NULL, token);
    		unsigned int findHash;
			findHash = atoi(cmd);
			result = find(findHash);

    		while (result == 0) {
    			result = find(findHash);
    		}

    		sprintf(reply, "%d", result);

    		if (send(rqst, reply, MAX_MSG_LENGTH, 0) < 0) {
    			perror("Falha no envio (send)!");
        		return NULL;
    	   	}
    	} else if (!strcmp(cmd, "query")) {
    		while ((recv(rqst, msg, MAX_MSG_LENGTH, 0)) > 0) {
    			unsigned int mHash;
    			// mHash = hash(msg);
    			mHash = atoi(msg);
    			result = find(mHash);

    			while (result == 0) {
    				result = find(mHash);
    			}

    			sprintf(reply, "%d", result);

    			if (send(rqst, reply, MAX_MSG_LENGTH, 0) < 0) {
    				perror("Falha no envio (send)!");
    				return NULL;
    			}
    		}
    	} else if (!strcmp(cmd, "stable")) {
    		memset(reply, 0, MAX_MSG_LENGTH);
    		
    		if (pre == NULL) {
    			sprintf(reply, "0");
    		} else {
    			sprintf(reply, "%d", pre->port);
    		}

    		if (send(rqst, reply, MAX_MSG_LENGTH, 0) < 0) {
    			perror("Falha no envio (send)!");
    			return NULL;
    	    }
    		break;
    	} else if(!strcmp(cmd, "notify")) {
    		cmd = strtok(NULL, token);
    		result = atoi(cmd);
    		
    		int inicio;
    		int fim;

    		if (pre == NULL) {
    			pre = (pcNode) malloc(sizeof(cNode));
    			pre->port = result;
    			pre->key = get_hash(pre->port);
    		} else if ((pre->key < node.key) && (pre->key < get_hash(result)) && (get_hash(result) < node.key)) {
    			pre->port = result;
    			pre->key = get_hash(pre->port);
    		} else if ((pre->key > node.key) &&
    				((get_hash(result) > pre->key) || (get_hash(result) < node.key))) {
				pre->port = result;
    			pre->key = get_hash(pre->port);
    		}

			inicio = node.key;
			fim = pre->key;

			while (inicio != fim) {
				inicio = (inicio + 1) % 128;
				if (chaves[inicio]) {
					memset(reply, 0, MAX_MSG_LENGTH);
					sprintf(reply, "%d", inicio);
					if (send(rqst, reply, MAX_MSG_LENGTH, 0) < 0) {
						perror("Falha no envio (send)!");
						return NULL;
					}
					chaves[inicio] = 0;
				}
			}

			memset(reply, 0, MAX_MSG_LENGTH);
			sprintf(reply, "%d", -1);
			if (send(rqst, reply, MAX_MSG_LENGTH, 0) < 0) {
				perror("Falha no envio (send)!");
				return NULL;
			}

    	} else if (!strcmp(cmd, "keep-alive")) {
    		cmd = strtok(NULL, token);
    		result = atoi(cmd);
    		suc2->port = result;
    		suc2->key = get_hash(suc2->port);
    	} else if (!strcmp(cmd, "reset-pre")) {
    		free(pre);
    		pre = NULL;
    	} else if (!strcmp(cmd, "insere-no")) {
    		while ((recv(rqst, msg, MAX_MSG_LENGTH, 0)) > 0) {
	    		
				unsigned int mHash;
    			mHash = atoi(msg);
    			result = find(mHash);

	    		while (result == 0) {
	    			result = find(mHash);
	    		}

	    		sprintf(reply, "%d", result);
	    		if (node.port == result) {
	    			chaves[mHash] = 1;
	    		} else {
	    			int sock;
					struct sockaddr_in server_addr;
					char msg[MAX_MSG_LENGTH], reply[MAX_MSG_LENGTH];

					if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
						perror("Nao foi possivel a criacao do socket!");
					    return 0;
					}

					server_addr.sin_addr.s_addr = inet_addr(addr);
					server_addr.sin_family = AF_INET;
					server_addr.sin_port = htons(result);

					if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
						perror("Falha para conectar a uma porta via socket!");
						return 0;
					}

					memset(msg, 0, sizeof(msg));
					sprintf(msg, "insere-no %u", mHash);

					if (send(sock, msg, MAX_MSG_LENGTH, 0) < 0) {
					    perror("Falha no envio (send)!");
					    return 0;
					}

					memset(reply, 0, sizeof(reply));
					if (recv(sock, reply, MAX_MSG_LENGTH, 0) < 0) {
						perror("Recv error: ");
						return 0;
					}
	    		}

	    		if (send(rqst, reply, MAX_MSG_LENGTH, 0) < 0) {
	    			perror("Falha no envio (send)!");
	        		return NULL;
	    	   	}
    	   	}
    	} else if (!strcmp(cmd, "remove-no")) {
    		while ((recv(rqst, msg, MAX_MSG_LENGTH, 0)) > 0) {
	    		
				unsigned int mHash;
    			mHash = atoi(msg);
    			result = find(mHash);

	    		while (result == 0) {
	    			result = find(mHash);
	    		}

	    		sprintf(reply, "%d", result);
	    		
	    		if (node.port == result) {
	    			chaves[mHash] = 0;
	    		} else {
	    			int sock;
					struct sockaddr_in server_addr;
					char msg[MAX_MSG_LENGTH], reply[MAX_MSG_LENGTH];

					if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
						perror("Nao foi possivel a criacao do socket!");
					    return 0;
					}

					server_addr.sin_addr.s_addr = inet_addr(addr);
					server_addr.sin_family = AF_INET;
					server_addr.sin_port = htons(result);

					if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
						perror("Falha para conectar a uma porta via socket!");
						return 0;
					}

					memset(msg, 0, sizeof(msg));
					sprintf(msg, "insere-no %u", mHash);

					if (send(sock, msg, MAX_MSG_LENGTH, 0) < 0) {
					    perror("Falha no envio (send)!");
					    return 0;
					}

					memset(reply, 0, sizeof(reply));
					if (recv(sock, reply, MAX_MSG_LENGTH, 0) < 0) {
						perror("Recv error: ");
						return 0;
					}
	    		}

	    		if (send(rqst, reply, MAX_MSG_LENGTH, 0) < 0) {
	    			perror("Falha no envio (send)!");
	        		return NULL;
	    	   	}
    	   	}
    	} else if (!strcmp(cmd, "adiciona-no")) {
    		printf("\nRecebendo adicao\n\n");
    		cmd = strtok(NULL, token);
    		result = atoi(cmd);
    		
    		chaves[get_hash(result)] = 1;
    	}/* else if (!strcmp(cmd, "elimina-no")) {
    		printf("\nRecebendo eliminacao. \n\n");
    		cmd = strtok(NULL, token);
    		result = atoi(cmd);
    		
    		chaves[get_hash(result)] = 0;
    	}*/
    	memset(msg, 0, sizeof(msg));
	}
	close(rqst);
	pthread_exit(0);
}

int main(int argc, char *argv[]) {
	void *self;

	node.key = 0;
	node.port = 0;

	suc = (pcNode) malloc(sizeof(cNode));
	suc2 = (pcNode) malloc(sizeof(cNode));
	pre = NULL;

	char *cmd;
	char *part;
	char *token = " ";
	
	int joinPort;

	for (int i; i < 128; ++i) {
		chaves[i] = 0;
	}

	cmd = (char*) malloc(512*sizeof(char));
	part = (char*) malloc(512*sizeof(char));

	// printf("Input command:\n");
	printf("Entre com o comando:\n");
	fgets(cmd, 512, stdin);

	part = strtok(cmd, token);
	part = strtok(NULL, token);

	node.port = atoi(part);
	node.key = get_hash(node.port);

	part = strtok(NULL, token);

	// Create Finger Table
	int i;
	for (i = 0; i < MBITS; i++) {
		fingerTable[i].start = node.key + pow(2, i);
		fingerTable[i].nodeInfo = (pcNode) malloc(sizeof(cNode));
		fingerTable[i].nodeInfo->port = node.port;
		fingerTable[i].nodeInfo->key = get_hash(fingerTable[i].nodeInfo->port);
	}
	suc2->port = node.port;
	suc2->key = get_hash(suc2->port);

	if (part == NULL) {
		newNode();
	} else {
		part = strtok(NULL, token);
		joinPort = atoi(part);
		joinNode(joinPort);
	}

	int sock;
	int rqst;

	socklen_t sockLen;

	struct sockaddr_in my_addr;
	struct sockaddr_in client_addr;

	sockLen = sizeof(client_addr);

	pthread_t pthreadID;

	memset((char *) &my_addr, 0, sizeof(my_addr));

	my_addr.sin_family = AF_INET;
	my_addr.sin_addr.s_addr = INADDR_ANY;
	my_addr.sin_port = htons(node.port);

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Nao foi possivel a criacao do socket!");
		return 1;
	}

	int true = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &true, sizeof(int));
	
	if ((bind(sock, (struct sockaddr *) &my_addr, sizeof(my_addr))) < 0) {
		perror("Falha na amarracao (bind) do socket!");
		return 1;
	}

	if (listen(sock, 5) < 0) {
		perror("Falha ao ouvir (listening) pelo socket!");
		return 1;
	}

	pthread_create(&pthreadID, NULL, update, NULL);

	pthread_create(&pthreadID, NULL, command, NULL);

	pthread_create(&pthreadID, NULL, print_node, NULL);

	while (1) {
    	if ((rqst = accept(sock, (struct sockaddr *) &client_addr, &sockLen)) < 0) {
    		perror("Falha para aceitar (accept) requisicoes sockets!");
    		continue;
    	}

    	int *pSock;
    	
    	pSock = (int *) malloc(sizeof(int));
    	*pSock = rqst;
    	
    	pthread_create(&pthreadID, NULL, chordNode, (void *) pSock);
    	pthread_join(pthreadID, &self);
	}
	close(sock);
	return 0;
}

void newNode() {
	pre = (pcNode) malloc(sizeof(cNode));
	suc->key = node.key;
	suc->port = node.port;
	pre->key = node.key;
	pre->port = node.port;
}

void joinNode(int joinPort) {
	char msg[MAX_MSG_LENGTH], reply[MAX_MSG_LENGTH];

	memset(msg, 0, sizeof(msg));
	memset(reply, 0, sizeof(msg));

	printf("Juntando a uma rede anel do tipo Chord existente!\n");
	// printf("Joining the Chord ring.\n");

	sprintf(msg, "join %d", node.port);

	int sock;

	struct sockaddr_in server_addr;

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Nao foi possivel a criacao do socket!");
		return;
	}

	server_addr.sin_addr.s_addr = inet_addr(addr); // char *addr="127.0.0.1";
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(joinPort);

	if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
		perror("Falha para conectar a uma porta via socket!");
	    return;
	}

	if (send(sock, msg, sizeof(msg), 0) < 0) {
		perror("Falha no envio (send)!");
		return;
	}

	fflush(stdin);

    if ((read(sock, reply, MAX_MSG_LENGTH)) < 0) {
    	perror("Recv error: ");
	   	return;
	}

    suc->port = atoi(reply);
	suc->key = get_hash(suc->port);

	//fflush(stdin);
    if ((read(sock, reply, MAX_MSG_LENGTH)) < 0) {
    	perror("Recv error: ");
	   	return;
	}
	int indice = atoi(reply);
	while (indice != -1) {
		//printf("\nindice: %d\n\n", indice);
		chaves[indice] = 1;

		//fflush(stdin);
	    if ((read(sock, reply, MAX_MSG_LENGTH)) < 0) {
	    	perror("Recv error: ");
		   	return;
		}
		indice = atoi(reply);
	}

	close(sock);
}

void *print_node() {
	while (1) {
		sleep(5);
		if (pre == NULL) continue;
		printf("--------------------------------------------------------------------------------");
		printf("Porta: %d\n", node.port);
		printf("Identificador do no: %d\n", node.key);
		printf("Sucessor: IP 127.0.0.1, porta %d, identificador %d.\n", suc->port, suc->key);
		printf("Predecessor: IP 127.0.0.1, porta %d, identificador %d.\n", pre->port, pre->key);
		printf("Finger Table: ");
		for (int i = 0; i < MBITS; i++) {
			if (i < MBITS-1)
				printf("%d, ", fingerTable[i].nodeInfo->key);
			else
				printf("%d.\n", fingerTable[i].nodeInfo->key);
		}
		printf("Chaves: ");
		
		int primeiro = 1;
		if (pre->key == node.key) {
			for (int i = 0; i < 128; i++) {
				if (chaves[i]) {
					if (primeiro) {
						printf("%d", i);
						primeiro = 0;
					} else {
						printf(", %d", i);
					}
				}
			}
		} else {
			int indice = pre->key;
			while (indice != node.key) {
				indice = (indice + 1) % 128;
				if (chaves[indice]) {
					if (primeiro) {
						printf("%d", indice);
						primeiro = 0;
					} else {
						printf(", %d", indice);
					}
				}
			}	
		}
		printf("\n");
		printf("Insira 'remover' para remover o no.\n");
	}
}
