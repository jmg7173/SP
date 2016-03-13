#include "20141578.h"
#include "shell_command.h"
#include "opcode.h"
#include "memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// In main, get command and call corresponding functions.
int main(){
	char command[MAX_COMMAND_LENGTH + 1];
	char command_copy[MAX_COMMAND_LENGTH + 1];
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

			// If command length is above than MAX_COMMAND_LENGTH, ignore it.
			if(command_length >= MAX_COMMAND_LENGTH)
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
}

// Delete trailing whitespaces
void delete_trailing_whitespace(char* str){
	int trailing_whitespace = 0;
	int i, len;
	int idx;
	int first_char = 0;

	len = strlen(str);
	for(i = 0; i <= len; i++){
		// It i reached at string length, check if it contains whitespaces at the end of string. 
		if(i == len){
			// If it contains whitespaces at the end part of string, to ignore whitespaces, put '\0' at the end of real string.
			if(trailing_whitespace >= 1){
				str[i-trailing_whitespace] = '\0';
			}
		}

		// If a character is whitespace(' ', '\t'), increase whitespace flag.
		// If whitespace is '\t', convert it to ' '
		if(str[i] == ' ' || str[i] == '\t'){
			trailing_whitespace++;
			if(str[i] == '\t') str[i] = ' ';
		}
		// If a character is not whitespace, do this things.
		else{
			// If trailing whitespace is more than 1, do this thing.
			if(trailing_whitespace >= 1){
				// A character is first meeting character, pull every characters from index to at front of string.
				if(!first_char)
					strncpy(str+idx, str+i, len-i+1);
				else if(str[i] == ',')
					strncpy(str+idx, str+i, len-i+1), i--;
				// If whitespace flag is above than 1, to delete whitespace, pull characters.
				else if(trailing_whitespace > 1)
					strncpy(str+idx+1, str+i, len-i+1);
				
				// Add NULL character end of new string.
				str[idx+len-i+1] = '\0';

				// Renew length of string.
				len = strlen(str);
				// Renew index i
				if(str[i] == ',') i = idx - 1;
				else if(trailing_whitespace > 1 || !first_char) i = idx;
			}

			// If a character is ',', do this things.
			if(str[i] == ','){
				// Add a space behind of ','
				if(i+1 < MAX_COMMAND_LENGTH && (str[i+1] != ' ' && str[i+1] != '\t')){
					char* tmp = (char*)malloc((len-i)*sizeof(char));
					strcpy(tmp, str+i+1);
					str[i+1] = ' ';
					str[i+2] = '\0';
					strcat(str,tmp);
					len = strlen(str);
					i--;
				}
			}
			//Initialize whitespace flag and first character flag.
			trailing_whitespace = 0;
			first_char = 1;
		}

		// If it is first trailing whitespace, set idx to i
		if(trailing_whitespace == 1){
			idx = i;
		}
	}
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
	
	// If it can't find command that matches above, return 1
	return 1;
}

// Print error message for each status.
int print_error(char* command, int status){
	switch(status){
		case 0: return 0;
		case 1: 
						fprintf(stderr, "Error : %s : Command not found\n",command); 
						break;
		case 2:
						fprintf(stderr, "Error : Missing address.\n");
						break;
		case 3:
						fprintf(stderr, "Error : Invalid address. Address range : 00000~FFFFF\n");
						break;
		case 4:
						fprintf(stderr, "Error : Invalid value. Value range : 00~FF\n");
						break;
		case 5:
						fprintf(stderr, "Error: Missing Value.\n");
						break;
		case 6:
						fprintf(stderr, "Error : Too much command.\n");
						break;
		case 7:
						fprintf(stderr, "Error : There is no command\n");
						break;
		case 8:
						fprintf(stderr, "Error : Start address must be smaller than end address.\n");
						break;
		case 9:
						fprintf(stderr, "Error : Please check number of comma(',').\n");
						break;
		case 10:
						fprintf(stderr, "Error : Missing mnemonic.\n");
						break;
		case 11:{
							char* tmp;
							tmp = strtok(command, " ");
							tmp = strtok(NULL, " ");
							fprintf(stderr, "Error : %s is not mnemonic\n", tmp);
							break;
						}
	}
	return 1;
}
