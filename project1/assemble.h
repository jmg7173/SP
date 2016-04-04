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
	int objectcode;
	unsigned char *obj_byte;
	int byte_len;
}assemble_table;

typedef struct TABLE{
	char symbol[MAX_INSTRUCTION];
	int line;
	int loc;
	struct TABLE *next;
}symbol_table;

FILE* open_file(int*, char*);
void command_symbol();
int command_type();
int command_assemble();
assemble_table line_to_command(char*, int*, int*, int);
symbol_table* find_at_symbol(const char*);
void add_at_tmp_symbol(const char*, int, int);
void add_at_symbol_table(const char*, int, int);
void delete_at_symbol_table();
void delete_at_tmp_symbol(int);

void create_objectcode(assemble_table *commands, int line, int *error);
int bit_reg(char reg);
char* make_lst(assemble_table *commands, int line, char *filename);
char* make_obj(assemble_table* commands, int line, char* filename, int start, int end);
int size_in_a_line(assemble_table* commands, int start, int line);

#endif
