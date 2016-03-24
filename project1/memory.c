#include "memory.h"
#include "constant.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Declare memory used for sicsim.
char memory[MAX_MEMORY] = {0};

// Command about dump
int command_dump(){
	char* tmp;
	static int curr_addr = 0; // Save next address that used for dump command.
	int start, end;
	int comma_checker; // Save numbers of comma
	
	tmp = strtok(NULL, " ");
	
	// If there is no other commands, execute command "dump"
	if(!tmp){
		// Print curr_addr + 159 (Because it includes itself)
		print_memory(curr_addr, curr_addr + DEFAULT_READ_MEMORY);
		curr_addr = curr_addr + DEFAULT_READ_MEMORY + 1;
		// After call function, if curr_addr is above than 0xFFFFF, set curr_addr to 0
		if(curr_addr > 0xFFFFF) curr_addr = 0;
		return 0;
	}
	
	// If next command exsits, check if it is hexadecimal number and is ranged in 00000~FFFFF and has just one or less comma.
	if((start = is_hexa(tmp, 1)) == -1)
		return 3;
	else if(start == -2)
		return 9;
	// Save number of comma
	comma_checker = comma_check(tmp);
	
	tmp = strtok(NULL, " ");
	// If next command doesn't exist, execue command "dump start"
	if(!tmp){
		// If there exists comma, it is invalid command.
		if(comma_checker) return 9;
		// Print start + 159
		print_memory(start, start+DEFAULT_READ_MEMORY);
		// Same as above.
		curr_addr = start + DEFAULT_READ_MEMORY + 1;
		if(curr_addr > 0xFFFFF) curr_addr = 0;
		return 0;
	}
	
	// Next command exists.
	// If it doesn't have just one comma, it is invalid.
	if(comma_checker != 1) return 9;
	// Check if it is valid command, process is same as above.
	if((end = is_hexa(tmp, 1)) == -1)
		return 3;
	else if(end == -2)
		return 9;

	// If there is comma, it is invalid.
	if(comma_check(tmp)) return 9;
	
	// If there is another command, it is invalid.
	if((tmp = strtok(NULL, " ")))
		return 6;
	
	// If start is larger than end, it is invalid.
	if(start > end)
		return 8;
	
	// Call function.
	print_memory(start, end);
	curr_addr = end + 1;
	if(curr_addr > 0xFFFFF) curr_addr = 0;
	
	// Valid Command : return value is 0
	return 0;
}

// Command about edit
int command_edit(){
	char* tmp;
	int addr, value;
	int comma_checker;

	tmp = strtok(NULL, " ");
	// If there is no command, it is invalid.
	if(!tmp)
		return 2;
	// If there is a command, check if it is right value, process is same as above.
	else if((addr = is_hexa(tmp, 1)) == -1)
		return 3;
	else if(addr == -2)
		return 9;
	comma_checker = comma_check(tmp);

	tmp = strtok(NULL, " ");
	// If there is no command, it is invalid.
	if(!tmp)
		return 5;
	
	// If comma is not just one, it is invalid. 
	if(comma_checker != 1)
		return 9;
	// Check if is right value (range : 00 ~ FF)
	if((value = is_hexa(tmp, 2)) == -1)
		return 4;
	else if(value == -2)
		return 9;

	// If there is comma, it is invalid.
	if(comma_check(tmp))
		return 9;
	
	// If there is additional command, it is invalid.
	if((tmp = strtok(NULL, " ")))
		return 6;

	// Set value at that address.
	memory[addr] = value;
	return 0;
}

// Command about fill
int command_fill(){
	char* tmp;
	int start, end, value;
	int comma_checker;
	
	tmp = strtok(NULL, " ");
	// If there is no command, it is invalid.
	if(!tmp)
		return 2;
	// If there is a command, check if it is right value, process is same as above.
	else if((start = is_hexa(tmp, 1)) == -1)
		return 3;
	else if(start == -2)
		return 9;
	comma_checker = comma_check(tmp);
	
	tmp = strtok(NULL, " ");
	// If there is no command, it is invalid.
	if(!tmp)
		return 2;
	// If there is not just one comma, it is invalid.
	if(comma_checker != 1)
		return 9;
	// Check if it is valid
	if((end = is_hexa(tmp, 1)) == -1)
		return 3;
	else if(end == -2)
		return 9;
	comma_checker = comma_check(tmp);
	
	// Process is same as above.
	tmp = strtok(NULL, " ");
	if(!tmp)
		return 5;
	if(comma_checker != 1)
		return 9;
	else if((value = is_hexa(tmp, 2)) == -1)
		return 4;
	else if(value == -2)
		return 9;

	// If there is comma, it is invalid.
	if(comma_check(tmp))
		return 9;
	
	// If there is additional command, it is invalid.
	if((tmp = strtok(NULL, " ")))
		return 6;
	
	// If start is larger than end, it is invalid.
	if(start > end)
		return 8;

	// Fill memory
	memset(memory+start, value, end-start+1);
	return 0;
}

// Check if given string is hexadecimal
// If option == 1, range : 0x00000~0xFFFFF
// If option == 2, range : 0x00~0xFF
int is_hexa(const char* str, int option){
	char **wrong = NULL;
	int length = strlen(str);
	int comma_check = 0;
	int i = 0;
	int result;
	int zero_idx;

	while(str[i] == '0') i++;
	zero_idx = i;
	
	//check if number is over integer range.
	if(strlen(str+zero_idx) > 8)
		return -1;
	
	// Check if there is comma and character is in hexadecimal character.
	for(i = 0; i < length; i++){
		if(str[i] == ',') comma_check++;
		else if(!is_in_range(str[i]))
			return -1;
	}
	
	// If comma has above than 1, it is invalid.
	if(comma_check > 1) return -2;
	
	// Convert string to hexadecimal number
	result = strtol(str,wrong,16);
	
	// Check range of hexadecimal number
	if(result < 0 || result > 0xFFFFF)
		return -1;

	// If option is 2, range is 00 ~ FF
	else if(option == 2 && (result < 0 || result > 0xFF))
		return -1;

	return result;
}

// Check if given character is in range of 0~9 or a~f or A~F
// 0 : Valid, 1 : Invalid.
int is_in_range(const char c){
	if( (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))
		return 1;
	return 0;
}

// Check number of comma
int comma_check(const char* str){
	int length = strlen(str);
	int i;
	int comma_check = 0;
	for(i = 0; i < length; i++)
		if(str[i] == ',') comma_check++;
	
	return comma_check;
}

// Function for command "dump"
void print_memory(int start, int end){
	int head_addr;
	int idx;
	int i;

	// If end is more than MAX_MEMORY, set end as MAX_MEMORY - 1
	if(end >= MAX_MEMORY)
		end = MAX_MEMORY-1;

	// Print Memory
	for(head_addr = start/16 * 16; head_addr < end/16 * 16 + 16; head_addr += 16){
		idx = head_addr;
		// Print Line
		printf("%05X ",head_addr);
		
		// Print hexadecimal number
		for(i = 0; i<16; i++, idx++){
			// If it is not in range, print blank
			if(idx < start || idx > end)
				printf("  ");
			// If it is in range, print hexadecimal
			else printf("%02X",memory[idx]);
			printf(" ");
		}
		printf("; ");

		// Print character
		idx = head_addr;
		for(i = 0; i<16; i++, idx++){
			// If it is not in printing memory ranage, print '.'
			if(idx < start || idx > end)
				printf(".");
			// If it is in printing range and hexadecimal number is in range 0x20~0x7E, print character
			else if(memory[idx] >= 0x20 && memory[idx] <= 0x7E)
				printf("%c",memory[idx]);
			// If it is not, print '.'
			else
				printf(".");
		}
		printf("\n");
	}
}

// reset memory to 0('\0', NULL)
void reset_memory(){
	memset(memory,0,MAX_MEMORY);
}
