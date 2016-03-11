#include "memory.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char memory[MAX_MEMORY] = {0};
int curr_addr = 0;

int command_dump(){
	char* tmp;
	int start, end;
	int comma_checker;
	
	tmp = strtok(NULL, " ");
	
	if(!tmp){
		print_memory(curr_addr, curr_addr + DEFAULT_READ_MEMORY);
		curr_addr = curr_addr + DEFAULT_READ_MEMORY + 1;
		if(curr_addr > 0xFFFFF) curr_addr = 0;
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
		if(curr_addr > 0xFFFFF) curr_addr = 0;
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
	if(curr_addr > 0xFFFFF) curr_addr = 0;
	
	return 0;
}

int command_edit(){
	char* tmp;
	int addr, value;
	int comma_checker;

	tmp = strtok(NULL, " ");
	if(!tmp)
		return 2;
	else if((addr = is_hexa(tmp, 1)) == -1)
		return 3;
	else if(addr == -2)
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
	memory[addr] = value;
	return 0;
}

int command_fill(){
	char* tmp;
	int start, end, value;
	int comma_checker;

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

//if option == 1, range : 0x00000~0xFFFFF
//if option == 2, range : 0x00~0xFF
int is_hexa(const char* str, int option){
	char **wrong = NULL;
	int length = strlen(str);
	int comma_check = 0;
	int i;
	int result;

	for(i = 0; i < length; i++){
		if(str[i] == ',') comma_check++;
		else if(!is_in_range(str[i]) || comma_check >= 1)
			return -1;
	}
	
	if(comma_check > 1) return -2;
	
	result = strtol(str,wrong,16);
	if(result > 0xFFFFF)
		return -1;
	else if(option == 2 && result > 0xFF)
		return -1;

	return result;
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

void reset_memory(){
	memset(memory,0,MAX_MEMORY);
}
