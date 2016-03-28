#ifndef _FILE_PROCESSING_H_
#define _FILE_PROCESSING_H_

#include <stdio.h>
#include "constant.h"

typedef struct{
	char symbol[MAX_INSTRUCTION];
	char mnemonic[MAX_INSTRUCTION];
	int loc;
	int line;
	int opcode;
	int format;
	char param1[MAX_STR_LENGTH];
	char reg1;
	char reg2;
	int use_X;
}assemble_table;

typedef struct TABLE{
	char symbol[MAX_INSTRUCTION];
	int line;
	int loc;
	struct TABLE *next;
}symbol_table;

FILE* open_file(int*, char*);
int command_symbol();
int command_type();
int command_assemble();
assemble_table line_to_command(char*, int*, int*, int, symbol_table*);
symbol_table* find_at_symbol(const char*, symbol_table*);
void add_at_symbol(const char*, int, int, symbol_table*);

#endif
