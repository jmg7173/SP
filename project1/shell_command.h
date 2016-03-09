#ifndef _SHELL_COMMAND_H
#define _SHELL_COMMAND_H

#define HELP_COMMAND_NUM 10
#define MAX_COMMAND_LENGTH 511

typedef struct _NODE{
	int idx;
	char command[MAX_COMMAND_LENGTH + 1];
	struct _NODE* next;
}history_node;
history_node* history_linked_list;
history_node* history_head;

extern char memory[];
extern int curr_addr;
const static char *help_commands[HELP_COMMAND_NUM] = {
	"h[elp]",
	"d[ir]",
	"q[uit]",
	"hi[story]",
	"du[mp] [start, end]",
	"e[dit] address, value",
	"f[ill] start, end, value",
	"reset",
	"opcode mnemonic",
	"opcodelist"
};


void print_help_commands();
void print_curr_directory();
void print_history();
void add_to_history(const char* str);
void delete_history_linked_list();

#endif
