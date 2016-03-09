#include "opcode.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define ROT32(x, y) ((x << y) | (x >> (32 - y)))
hash_node *hash_table[MAX_HASH_BUCKET] = {0};
static unsigned int seed;
void make_hashtable(){
	FILE* fp = fopen("opcode.txt","r");
	char tmp[100];
	int opcode, format1, format2;
	char instr[MAX_INSTRUCTION];
	srand(time(NULL));
	seed = rand();
	while(fgets(tmp,99,fp) != NULL){
		sscanf(tmp,"%02X %s %d/%d",&opcode,instr,&format1,&format2);
		set_hashtable(instr,opcode,format1,format2);
	}
}

int search_mnemonic(const char* key){
	int hash = hashfunction_murmur(key);
	hash_node *tmp = hash_table[hash];
	while(tmp != NULL){
		if(strcmp(key, tmp->instr) == 0)
			return tmp->opcode;
		tmp = tmp->next;
	}
	return -1;
}

void set_hashtable(char* key, int value, int format1, int format2){
	int hash = hashfunction_murmur(key);
	if(hash_table[hash] == NULL){
		hash_table[hash] = (hash_node*)malloc(sizeof(hash_node));
		strcpy(hash_table[hash]->instr,key);
		hash_table[hash]->opcode = value;
		hash_table[hash]->format1 = format1;
		hash_table[hash]->format2 = format2;
		hash_table[hash]->next = NULL;
	}
	else{
		hash_node* new_node = (hash_node*)malloc(sizeof(hash_node));
		hash_node* tmp = hash_table[hash];
		while(tmp->next != NULL)
			tmp = tmp->next;
		tmp->next = new_node;
		new_node->next = NULL;
		strcpy(new_node->instr,key);
		new_node->opcode = value;
		new_node->format1 = format1;
		new_node->format2 = format2;
	}
}

unsigned int hashfunction_murmur(const char* key){
	static const unsigned int c1 = 0xcc9e2d51;
	static const unsigned int c2 = 0x1b873593;
	static const unsigned int r1 = 15;
	static const unsigned int r2 = 13;
	static const unsigned int m = 5;
	static const unsigned int n = 0xe6546b64;

	unsigned int hash = seed;
	unsigned int len = strlen(key);
	const int nblocks = len / 4;
	const unsigned int *blocks = (const unsigned int *) key;
	int i;
	unsigned int k;
	const unsigned char *tail = (const unsigned char *) (key + nblocks * 4);
	unsigned int k1 = 0;


	for (i = 0; i < nblocks; i++) {
		k = blocks[i];
		k *= c1;
		k = ROT32(k, r1);
		k *= c2;

		hash ^= k;
		hash = ROT32(hash, r2) * m + n;
	}

	switch (len & 3) {
		case 3:
			k1 ^= tail[2] << 16;
		case 2:
			k1 ^= tail[1] << 8;
		case 1:
			k1 ^= tail[0];

			k1 *= c1;
			k1 = ROT32(k1, r1);
			k1 *= c2;
			hash ^= k1;
	}

	hash ^= len;
	hash ^= (hash >> 16);
	hash *= 0x85ebca6b;
	hash ^= (hash >> 13);
	hash *= 0xc2b2ae35;
	hash ^= (hash >> 16);

	return hash % MAX_HASH_BUCKET;
}

void print_opcode(){
	int i;
	hash_node* tmp;
	for(i = 0; i < MAX_HASH_BUCKET; i++){
		printf("%2d : ",i);
		tmp = hash_table[i];
		while(tmp != NULL){
			printf("[%s, %02X]",tmp->instr, tmp->opcode);
			if(tmp->next != NULL)
				printf(" -> ");
			tmp = tmp->next;
		}
		printf("\n");
	}
}
