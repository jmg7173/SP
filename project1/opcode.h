#ifndef _OPCODE_H_
#define _OPCODE_H_

#define MAX_INSTRUCTION 10
#define MAX_HASH_BUCKET 20

typedef struct _HASH_NODE{
	int opcode;
	char instr[MAX_INSTRUCTION];
	struct _HASH_NODE *next;
}hash_node;

hash_node *hash_table[MAX_HASH_BUCKET] = NULL;

void print_opcode();
#endif
