#ifndef _OPCODE_H_
#define _OPCODE_H_

#define MAX_INSTRUCTION 10
#define MAX_HASH_BUCKET 20

typedef struct _HASH_NODE{
	int opcode;
	char instr[MAX_INSTRUCTION];
	int format1;
	int format2;
	struct _HASH_NODE *next;
}hash_node;

void make_hashtable();
void set_hashtable(char*, int, int, int);
void delete_hashtable();
void print_opcode();
hash_node* search_mnemonic(const char*);
unsigned int hashfunction_murmur(const char*);
unsigned int my_hashfunction(const char*);
#endif
