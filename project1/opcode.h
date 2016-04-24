#ifndef _OPCODE_H_
#define _OPCODE_H_

#include "constant.h"
#include "structures.h"

void make_hashtable();
void set_hashtable(char*, int, int, int);
void set_opcodetable(char* instr, int value, int format1, int format2);
void delete_hashtable();
int command_opcode();
void print_opcode();
hash_node* search_mnemonic(const char*);
opcode_table search_as_opcode(const int value);
unsigned int hashfunction_murmur(const char*);
unsigned int my_hashfunction(const char*);

#endif
