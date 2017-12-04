#ifndef CHORD_H_
#define CHORD_H_

#define MAX_MSG_LENGTH 	2048
#define MBITS 			7

typedef struct _cNode *pcNode;

pcNode suc;
pcNode suc2;
pcNode pre;

typedef struct _fingerTable {
	unsigned int start;
	pcNode nodeInfo;
} fTable;

fTable fingerTable[MBITS];

typedef struct _cNode {
	unsigned int key;
	int port;
} cNode;

cNode node;

int chaves[128];

char *addr = "127.0.0.1";

unsigned int hash(char *key);

unsigned int get_hash(int port);

int find(unsigned int hashValue);

void fix_fingers(int i);

void notify();

void stabilize();

void newNode();

void joinNode(int joinPort);

void reset_pre();

void keep_alive();

void *update();

void *command();

void *print_node();

#endif /* CHORD_H_ */
