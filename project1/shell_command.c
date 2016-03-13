#include "shell_command.h"
#include <dirent.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *help_commands[HELP_COMMAND_NUM] = {
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

// Print current directory's file and directories.
void print_curr_directory(){
	DIR *dir; // For open current directory
	struct dirent *files; // For save file
	struct stat info; // For save file state as struct state
	mode_t state; // For save file state
	int enter_flag = 0;

	// Open current directory
	dir = opendir(".");
	// Read directory until there is no files to read
	while( (files = readdir(dir)) != NULL){
		// except current directory and previous directory(".", "..")
		if(strcmp(files->d_name,".") == 0 ||
				strcmp(files->d_name,"..") == 0)
			continue;
		enter_flag++;
		// Get file state as struct state
		stat(files->d_name, &info);
		// Get file state exactly chmod
		state = info.st_mode;
		// If it is directory, add '/'
		if(S_ISDIR(state))
			printf("%20s/\t",files->d_name);
		// If it is executable for user, add '*'
		else if(S_IXUSR & state)
			printf("%20s*\t", files->d_name);
		// If it is not directory and executable file, than just print filename
		else
			printf("%20s\t", files->d_name);

		if(enter_flag == 4){
			printf("\n");
			enter_flag = 0;
		}
	}
	printf("\n");
	closedir(dir);
}

// Print every valid commands
void print_help_commands(){
	int i;
	for(i = 0; i<HELP_COMMAND_NUM; i++)
		printf("%s\n",help_commands[i]);
}

// Add valid command to history as linked list
void add_to_history(const char* str){
	history_node* new_node;
	new_node = (history_node*)malloc(sizeof(history_node));
	strcpy(new_node->command, str);
	if(history_linked_list == NULL){
		new_node->idx = 1;
		history_linked_list = new_node;
		history_head = new_node; 
	}
	else{
		new_node->idx = history_linked_list->idx + 1;
		history_linked_list->next = new_node;
		history_linked_list = new_node;
		new_node->next = NULL;
	}
}

// Print history
void print_history(){
	history_node* tmp;
	tmp = history_head;
	while(tmp != NULL){
		printf("%-4d %s\n",tmp->idx, tmp->command);
		tmp = tmp->next;
	}
}

// Delete all of history
void delete_history_linked_list(){
	history_node* tmp;
	tmp = history_head;
	while(tmp != NULL){
		tmp = history_head->next;
		free(history_head);
		history_head = tmp;
	}
}
