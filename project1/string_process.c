#include "string_process.h"
#include "constant.h"
#include <stdlib.h>
#include <string.h>

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
				i = idx;
			}

			// If a character is ',', do this things.
			if(str[i] == ','){
				// Add a space behind of ','
				if(i+1 < MAX_STR_LENGTH && (str[i+1] != ' ' && str[i+1] != '\t')){
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


