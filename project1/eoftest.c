#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(){
	char tmp;
	while(1){
		if((tmp = getchar()) != '\n')
			printf("%c\n",tmp);
		else{
			printf("EOF\n");
			break;
		}
	}
	return 0;
}
