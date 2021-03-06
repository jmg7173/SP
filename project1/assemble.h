#ifndef _ASSEMBLE_H_
#define _ASSEMBLE_H_

#include <stdio.h>
#include "structures.h"


/**** command function ****/
int command_assemble();

/**** assemble function ****/
void create_objectcode(assemble_table *commands, int line, int *error);
char* make_lst(assemble_table *commands, int line, char *filename);
char* make_obj(assemble_table* commands, int line, char* filename, int start, int end);
int size_in_a_line(assemble_table* commands, int start, int line);
int obj_format34(assemble_table cmd, int calc_type, int pc, int b, int *error);
#endif
