#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct _hashnode{
	int opcode;
	char instr[10];
	struct _hashnode *next;
}hashnode;

hashnode *hashtable[20];

int main(){
	int i;
	for(i = 0; i<20; i++){
		hashtable[i] = (hashnode*)calloc(1,sizeof(hashnode));
		hashtable[i]->opcode = i;
	}
	for(i = 0; i<20; i++){
		printf("%d\n",hashtable[i]->opcode);
	}
	return 0;
}
