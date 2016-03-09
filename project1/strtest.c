#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(){
	char str[] = "      a      help     ,      sdfsdf        ddddd         ";
	int trailing_whitespace;
	int idx;
	int i;
	int len = strlen(str);
	int first_char = 0;
	
	printf("%s!\n",str);
	for(i = 0; i <= len; i++){
		if(i == len){
			if(trailing_whitespace >= 1){
				str[i-trailing_whitespace] = '\0';
			}
			break;
		}
		if(str[i] == ' ' || str[i] == '\t'){
			trailing_whitespace++;
			printf("tw : %d\n",trailing_whitespace);
			if(str[i] == '\t') str[i] = ' ';
		}
		else{
			if(trailing_whitespace >= 1){
				printf("before i : %d\n",i);
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
				printf("after i : %d\n",i);
				printf("%s!\n",str);
			}
			first_char = 1;
			trailing_whitespace = 0;
		}
		
		if(trailing_whitespace == 1){
			idx = i;
		}
	}
	printf("%s!\n",str);
	printf("help, !\n");
	return 0;
}
