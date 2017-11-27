#ifndef CHORD_H_
#define CHORD_H_

#define MAX_MSG_LENGTH 	2048
#define MBITS 			7

typedef struct _cNode * pcNode;

// pcNode suc;
// pcNode suc2;
// pcNode pre;

//typedef struct _fingerTable {
struct finger_table {
	unsigned int start;
	ft_node nodeInfo;
} finger;

fTable fingerTable[MBITS+1];

struct ft_node {
	unsigned int key;
	int port;
};

cNode localNode;

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
