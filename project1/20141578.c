#include "20141578.h"
#include "shell_command.h"
#include "opcode.h"
#include "memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(){
	char command[MAX_COMMAND_LENGTH + 1];
	char command_copy[MAX_COMMAND_LENGTH + 1];
	int tmp;
	int command_length;
	int is_error;
	
	make_hashtable();

	while(1){
		printf("sicsim> ");
		command_length = 0;
		is_error = 0;
		while((tmp=getchar()) != '\n'){
			if(tmp == EOF){
				quit_program();
				return 0;
			}
			if(command_length >= MAX_COMMAND_LENGTH)
				continue;
			command[command_length++] = (char)tmp;
		}
		command[command_length] = '\0';

		strcpy(command_copy, command);
		delete_trailing_whitespace(command_copy);
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
		else
			is_error = print_error(command, command_with_whitespace(command_copy));

		if(!is_error && 
				!(strcmp(command_copy, "hi") == 0 || strcmp(command_copy, "history") == 0)) 
			add_to_history(command);
	}
	return 0;
}

void quit_program(){
	delete_history_linked_list();
	delete_hashtable();
}

void delete_trailing_whitespace(char* str){
	int trailing_whitespace = 0;
	int i, len;
	int idx;
	int first_char = 0;

	len = strlen(str);
	for(i = 0; i <= len; i++){
		if(i == len){
			if(trailing_whitespace >= 1){
				str[i-trailing_whitespace] = '\0';
			}
		}
		if(str[i] == ' ' || str[i] == '\t'){
			trailing_whitespace++;
			if(str[i] == '\t') str[i] = ' ';
		}
		else{
			if(trailing_whitespace >= 1){
				if(!first_char)
					strncpy(str+idx, str+i, len-i+1);
				else if(str[i] == ',')
					strncpy(str+idx, str+i, len-i+1);
				else if(trailing_whitespace > 1)
					strncpy(str+idx+1, str+i, len-i+1);
				str[idx+len-i+1] = '\0';
				len = strlen(str);
				if(str[i] == ',') i = idx - 1;
				else i = idx;
			}
			else if(str[i] == ','){
				if(i+1 < MAX_COMMAND_LENGTH && (str[i+1] != ' ' && str[i+1] != '\t')){
					char* tmp = (char*)malloc((len-i)*sizeof(char));
					strcpy(tmp, str+i+1);
					str[i+1] = ' ';
					str[i+2] = '\0';
					strcat(str,tmp);
					len = strlen(str);
				}
			}
			trailing_whitespace = 0;
			first_char = 1;
		}
		if(trailing_whitespace == 1){
			idx = i;
		}
	}
}

int command_with_whitespace(char* str){
	char *tmp;
	
	tmp = strtok(str, " ");
		
	//Check if string is empty
	if(!tmp)
		return 7;

	//Compare with dump command
	else if(strcmp(tmp, "du") == 0 || strcmp(tmp, "dump") == 0)
		return command_dump();
		
	//Compare with edit command
	else if(strcmp(tmp, "e") == 0 || strcmp(tmp, "edit") == 0)
		return command_edit();

	//Compare with fill command
	else if(strcmp(tmp, "f") == 0 || strcmp(tmp, "fill") == 0)
		return command_fill();
	
	else if(strcmp(tmp, "opcode") == 0)
		return command_opcode();
	
	return 1;
}


int print_error(const char* command, int status){
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
		case 11:
						fprintf(stderr, "Error : Doesn't exist mnemonic.\n");
						break;
	}
	return 1;
}
