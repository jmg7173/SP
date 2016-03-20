#ifndef _SHELL_COMMAND_H
#define _SHELL_COMMAND_H

#define HELP_COMMAND_NUM 13
#define MAX_COMMAND_LENGTH 511

typedef struct _NODE{
	int idx;
	char command[MAX_COMMAND_LENGTH + 1];
	struct _NODE* next;
}history_node;
history_node* history_linked_list;
history_node* history_head;

void print_help_commands();
void print_curr_directory();
void print_history();
void add_to_history(const char* str);
void delete_history_linked_list();

#endif
