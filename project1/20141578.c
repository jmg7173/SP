#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "20141578.h"
#include "addr.h"
#include "assemble.h"
#include "constant.h"
#include "debug.h"
#include "error.h"
#include "file.h"
#include "loader.h"
#include "memory.h"
#include "opcode.h"
#include "run.h"
#include "shell_command.h"
#include "string_process.h"
#include "symbol.h"

// In main, get command and call corresponding functions.
int main(){
	char command[MAX_STR_LENGTH + 1];
	char command_copy[MAX_STR_LENGTH + 1];
	int tmp;
	int command_length;
	int is_error; // It represents that input command is valid or invalid.
	
	make_hashtable(); // Make hashtable associated with opcodes. 

	while(1){ // Run until input command is "q" or "quit"
		printf("sicsim> ");
		command_length = 0; // Initializing.
		is_error = 0;  // Initializing.
		while((tmp=getchar()) != '\n'){ // Get commands until it get '\n'
			if(tmp == EOF){ // For file redirection, if it meets EOF, quit program.
				quit_program();
				return 0;
			}

			// If command length is above than MAX_STR_LENGTH, ignore it.
			if(command_length >= MAX_STR_LENGTH)
				continue;
			command[command_length++] = (char)tmp;
		}
		// At the end of command parameter, put '\0'
		command[command_length] = '\0';

		// For preserve original string, copy it.
		strcpy(command_copy, command);

		// Delete trailing whitespace
		// a    ,     -> a,
		// dump      4    ,     28 -> dump 4, 28
		delete_trailing_whitespace(command_copy);
		
		// Call right functions for each command
		if(strcmp(command_copy, "q") == 0 || strcmp(command_copy, "quit") == 0){
			quit_program();
			break;
		}
		else if(strcmp(command_copy, "h") == 0 || strcmp(command_copy, "help") == 0)
			print_help_commands();
		else if(strcmp(command_copy, "d") == 0 || strcmp(command_copy, "dir") == 0)
			print_curr_directory();
		else if(strcmp(command_copy, "hi") == 0 || strcmp(command_copy, "history") == 0)
			add_to_history(command), print_history();
		else if(strcmp(command_copy, "reset") == 0)
			reset_memory();
		else if(strcmp(command_copy, "opcodelist") == 0)
			print_opcode();
		else if(strcmp(command_copy, "symbol") == 0)
			command_symbol();
		else if(strcmp(command_copy, "run") == 0)
			command_run();

		// If there is no such command, check if command is belonged in whitespace-needed commands.
		// Also there is no such command in that, print error message and save '1' at is_error.
		else
			is_error = print_error(command, command_with_whitespace(command_copy));

		// If input command is valid, save command at histroy.
		if(!is_error && 
				!(strcmp(command_copy, "hi") == 0 || strcmp(command_copy, "history") == 0)) 
			add_to_history(command);
	}
	return 0;
}

// This function works at quit program.
// freeing allocated memories.
void quit_program(){
	delete_history_linked_list();
	delete_hashtable();
	delete_at_symbol_table();
}

// Process associated with command that contains whitespace 
// Return value means error code. 0 : Valid, Others : Invalid.
int command_with_whitespace(char* str){
	char *tmp;
	
	// Tokenize string as ' '
	tmp = strtok(str, " ");
		
	// Check if string is empty
	if(!tmp)
		return 7;

	// Compare with dump command
	else if(strcmp(tmp, "du") == 0 || strcmp(tmp, "dump") == 0)
		return command_dump();
		
	// Compare with edit command
	else if(strcmp(tmp, "e") == 0 || strcmp(tmp, "edit") == 0)
		return command_edit();

	// Compare with fill command
	else if(strcmp(tmp, "f") == 0 || strcmp(tmp, "fill") == 0)
		return command_fill();
	
	// Compare with opcode command
	else if(strcmp(tmp, "opcode") == 0)
		return command_opcode();

	else if(strcmp(tmp, "type") == 0)
		return command_type();

	else if(strcmp(tmp, "assemble") == 0)
		return command_assemble();

	// Compare with linking loader command
	else if(strcmp(tmp, "progaddr") == 0)
		return command_progaddr();

	else if(strcmp(tmp, "loader") == 0)
		return command_loader();

	// Compare with debug command
	else if(strcmp(tmp, "bp") == 0)
		return command_bp();
	
	// If it can't find command that matches above, return 1
	return 1;
}
