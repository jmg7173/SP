#include "20141578.h"
#include "shell_command.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(){
	char command[MAX_COMMAND_LENGTH + 1];
	char command_copy[MAX_COMMAND_LENGTH + 1];
	int tmp;
	int command_length;

	while(1){
		printf("sicsim> ");
		command_length = 0;
		while((tmp=getchar()) != '\n'){
			if(tmp == EOF){
				delete_history_linked_list();
				return 0;
			}
			if(command_length >= MAX_COMMAND_LENGTH)
				continue;
			command[command_length++] = (char)tmp;
		}
		command[command_length] = '\0';

		add_to_history(command);
		// TODO : trailing whitespace?? trailing comma??
		strcpy(command_copy, command);
		delete_trailing_whitespace(command_copy);
		if(strcmp(command_copy, "q") == 0 || strcmp(command_copy, "quit") == 0){
			delete_history_linked_list();
			break;
		}
		else if(strcmp(command_copy, "h") == 0 || strcmp(command_copy, "help") == 0)
			print_help_commands();
		else if(strcmp(command_copy, "d") == 0 || strcmp(command_copy, "dir") == 0)
			print_curr_directory();
		else if(strcmp(command_copy, "hi") == 0 || strcmp(command_copy, "history") == 0)
			print_history();
		else if(strcmp(command_copy, "reset") == 0)
			memset(memory,0,MAX_MEMORY);
		else if(strcmp(command_copy, "opcodelist") == 0)
			print_opcode();
		else
			print_error(command, command_with_whitespace(command_copy));
	}
	return 0;
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
	int start, end, value;
	int comma_checker;
	
	tmp = strtok(str, " ");
	
	//Check if string is empty
	if(!tmp)
		return 7;

	//Compare with dump command
	else if(strcmp(tmp, "du") == 0 || strcmp(tmp, "dump") == 0){
		tmp = strtok(NULL, " ");
		if(!tmp){
			print_memory(curr_addr, curr_addr + DEFAULT_READ_MEMORY);
			curr_addr = curr_addr + DEFAULT_READ_MEMORY + 1;
			return 0;
		}

		if((start = is_hexa(tmp, 1)) == -1)
			return 3;
		else if(start == -2)
			return 9;
		comma_checker = comma_check(tmp);

		tmp = strtok(NULL, " ");
		if(!tmp){
			if(comma_checker) return 9;
			print_memory(start, start+DEFAULT_READ_MEMORY);
			curr_addr = start + DEFAULT_READ_MEMORY + 1;
			return 0;
		}
		if(comma_checker != 1) return 9;
		if((end = is_hexa(tmp, 1)) == -1)
			return 3;
		else if(end == -2)
			return 9;
		if(comma_check(tmp)) return 9;

		if((tmp = strtok(NULL, " ")))
			return 6;

		if(start > end)
			return 8;
		print_memory(start, end);
		curr_addr = end + 1;
		return 0;
	}
	
	//Compare with edit command
	else if(strcmp(tmp, "e") == 0 || strcmp(tmp, "edit") == 0){
		tmp = strtok(NULL, " ");
		if(!tmp)
			return 2;
		else if((start = is_hexa(tmp, 1)) == -1)
			return 3;
		else if(start == -2)
			return 9;
		comma_checker = comma_check(tmp);

		tmp = strtok(NULL, " ");
		if(!tmp)
			return 5;
		if(comma_checker != 1)
			return 9;
		if((value = is_hexa(tmp, 2)) == -1)
			return 4;
		else if(value == -2)
			return 9;
		if(comma_check(tmp))
			return 9;
		
		if((tmp = strtok(NULL, " ")))
			return 6;
		memory[start] = value;
		return 0;
	}
	
	//Compare with fill command
	else if(strcmp(tmp, "f") == 0 || strcmp(tmp, "fill") == 0){
		tmp = strtok(NULL, " ");
		if(!tmp)
			return 2;
		else if((start = is_hexa(tmp, 1)) == -1)
			return 3;
		else if(start == -2)
			return 9;
		comma_checker = comma_check(tmp);
		
		tmp = strtok(NULL, " ");
		if(!tmp)
			return 2;
		if(comma_checker != 1)
			return 9;
		if((end = is_hexa(tmp, 1)) == -1)
			return 3;
		else if(end == -2)
			return 9;
		comma_checker = comma_check(tmp);
		
		tmp = strtok(NULL, " ");
		if(!tmp)
			return 5;
		if(comma_checker != 1)
			return 9;
		else if((value = is_hexa(tmp, 2)) == -1)
			return 4;
		else if(value == -2)
			return 9;
		if(comma_check(tmp))
			return 9;
		
		if((tmp = strtok(NULL, " ")))
			return 6;
		
		if(start > end)
			return 8;
		memset(memory+start, value, end-start+1);
		return 0;
	}
	return 1;
}

//if option == 1, range : 0x00000~0xFFFFF
//if option == 2, range : 0x00~0xFF
int is_hexa(const char* str, int option){
	char **wrong = NULL;
	int length = strlen(str);
	int comma_check = 0;
	int i;

	if(option == 2){
		if(length > 3) return -1;
	}
	else{
		if(length > 6) return -1;
	}
	for(i = 0; i < length; i++){
		if(str[i] == ',') comma_check++;
		else if(!is_in_range(str[i]) || comma_check >= 1)
			return -1;
	}
	
	if(comma_check > 1) return -2;
	return strtol(str,wrong, 16);
}

int is_in_range(const char c){
	if( (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))
		return 1;
	return 0;
}

int comma_check(const char* str){
	int length = strlen(str);
	int i;
	int comma_check = 0;
	for(i = 0; i < length; i++)
		if(str[i] == ',') comma_check++;
	
	return comma_check;
}

void print_memory(int start, int end){
	int head_addr;
	int idx;
	int i;
	if(start >= MAX_MEMORY)
		start = MAX_MEMORY-1;
	if(end >= MAX_MEMORY)
		end = MAX_MEMORY-1;
	for(head_addr = start/16 * 16; head_addr < end/16 * 16 + 16; head_addr += 16){
		idx = head_addr;
		printf("%05X ",head_addr);
		for(i = 0; i<16; i++, idx++){
			if(idx < start || idx > end)
				printf("  ");
			else printf("%02X",memory[idx]);
			printf(" ");
		}
		printf("; ");

		idx = head_addr;
		for(i = 0; i<16; i++, idx++){
			if(idx < start || idx > end)
				printf(".");
			else if(memory[idx] >= 0x20 && memory[idx] <= 0x7E)
				printf("%c",memory[idx]);
			else
				printf(".");
		}
		printf("\n");
	}
}

void print_error(const char* command, int status){
	switch(status){
		case 0: break;
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
	}
}

void print_opcode(){
}
