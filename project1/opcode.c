#include "opcode.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// macro for rotate binary numbers for not losing any bit
#define ROT32(x, y) ((x << y) | (x >> (32 - y)))

// Declare hash_table parameter that has MAX_HASH_BUCKET buckets
hash_node *hash_table[MAX_HASH_BUCKET] = {0};

// Seed for hash function
static unsigned int seed = 23;

// Make hashtable
void make_hashtable(){
	FILE* fp = fopen("opcode.txt","r");
	char tmp[100];
	int opcode, format1, format2 = -1;
	char instr[MAX_INSTRUCTION];
	while(fgets(tmp,99,fp) != NULL){
		// Get information of opcode.
		// If it has two formats, save two formats.
		// If it has one format, save format2 as -1
		sscanf(tmp,"%02X %s %d/%d",&opcode,instr,&format1,&format2);
		set_hashtable(instr,opcode,format1,format2);
		format2 = -1;
	}
}

// Search mnemonic that saved at hashtable
// If it doesn't find return NULL
hash_node* search_mnemonic(const char* key){
	int hash = hashfunction_murmur(key); // To find address, use hash function
	hash_node *tmp = hash_table[hash];
	// Search mnemonic
	while(tmp != NULL){
		if(strcmp(key, tmp->instr) == 0)
			return tmp;
		tmp = tmp->next;
	}
	return NULL;
}

// Set mnemonic at hashtable
void set_hashtable(char* key, int value, int format1, int format2){
	int hash = hashfunction_murmur(key); // To set key, use hash function
	
	if(hash_table[hash] == NULL){ // If that hash table bucket doesn't allocated memory, allocate it.
		hash_table[hash] = (hash_node*)malloc(sizeof(hash_node));
		strcpy(hash_table[hash]->instr,key);
		hash_table[hash]->opcode = value;
		hash_table[hash]->format1 = format1;
		hash_table[hash]->format2 = format2;
		hash_table[hash]->next = NULL;
	}

	// If there is hash table bucket, put it in at tail
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

// Deallocate memories of hash table
void delete_hashtable(){
	int i;
	hash_node *tmp;
	for(i = 0; i<MAX_HASH_BUCKET; i++){
		tmp = hash_table[i];
		while(tmp != NULL){
			tmp = hash_table[i]->next;
			free(hash_table[i]);
			hash_table[i] = tmp;
		}
	}
}

// murmur2 hash function from http://en.wikipedia.org/wiki/MurmurHash
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

// Function for command opcode
int command_opcode(){
	hash_node* mnemonic;
	char* tmp;

	tmp = strtok(NULL, " ");
	if(!tmp)
		return 10;
	mnemonic = search_mnemonic(tmp);
	
	if((tmp = strtok(NULL," ")))
			return 6;
	if(mnemonic == NULL)
		return 11;
	
	else{ 
		printf("opcode is %02X ",mnemonic->opcode);
		if(mnemonic->format2 != -1)
			printf("format : %d, %d\n",mnemonic->format1,mnemonic->format2);
		else
			printf("format : %d\n",mnemonic->format1);
	}
	return 0;
}

// Print every opcode saved at hash table
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
