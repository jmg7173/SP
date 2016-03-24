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
