#ifndef _ASSEMBLE_H_
#define _ASSEMBLE_H_

#include <stdio.h>
#include "structures.h"


/**** command function ****/
FILE* open_file(int*, char*);
void command_symbol();
int command_type();
int command_assemble();

/**** assemble function ****/
assemble_table line_to_command(char*, int*, int*, int);
void create_objectcode(assemble_table *commands, int line, int *error);
char* make_lst(assemble_table *commands, int line, char *filename);
char* make_obj(assemble_table* commands, int line, char* filename, int start, int end);
int size_in_a_line(assemble_table* commands, int start, int line);
void set_command_table(assemble_table *table, char *mnemonic, char *param, int opcode, int format, int loc, int line);

/**** symbol function ****/
symbol_table* find_at_symbol(const char*);
void add_at_tmp_symbol(const char*, int, int);
void add_at_symbol_table(const char*, int, int);
void delete_at_symbol_table();
void delete_at_tmp_symbol();



#endif
