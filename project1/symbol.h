#ifndef _SYMBOL_H_
#define _SYMBOL_H_

#include "structures.h"

void command_symbol();
symbol_table* find_at_symbol(const char*);
void add_at_tmp_symbol(const char*, int, int);
void add_at_symbol_table(const char*, int, int);
void delete_at_symbol_table();
void delete_at_tmp_symbol(int);

#endif
