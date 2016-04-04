#ifndef _OPCODE_H_
#define _OPCODE_H_

#include "constant.h"
#include "structures.h"

void make_hashtable();
void set_hashtable(char*, int, int, int);
void delete_hashtable();
int command_opcode();
void print_opcode();
hash_node* search_mnemonic(const char*);
unsigned int hashfunction_murmur(const char*);
unsigned int my_hashfunction(const char*);

#endif
