#ifndef _SHELL_COMMAND_H
#define _SHELL_COMMAND_H

#include "constant.h"
#include "structures.h"

history_node* history_linked_list;
history_node* history_head;

void print_help_commands();
void print_curr_directory();
void print_history();
void add_to_history(const char* str);
void delete_history_linked_list();

#endif
