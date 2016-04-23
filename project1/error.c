#include "error.h"
#include <stdio.h>
#include <string.h>

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
		case 12:{
							char *tmp;
							tmp = strtok(command, " ");
							tmp = strtok(NULL, " ");
							fprintf(stderr, "Error : %s doesn't exist file.\n",tmp);
							break;
						}
		case 13:
						fprintf(stderr, "Error : Can't make immediate file.\n");
						break;
		case 14:
						fprintf(stderr, "Error : Can't load object files.\n");
		default:
						break;
	}
	return 1;
}

void error_in_assemble(int status, int line){
	switch(status){
		/**** Error for input ****/
		case 1:
			fprintf(stderr,"Error : line %d Too much parameter.\n",line);
			break;

		case 2:
			fprintf(stderr,"Error : line %d Need more parameter.\n",line);
			break;

		case 3:
			fprintf(stderr,"Error : line %d Additional parameter doesn't need.\n", line);
			break;

		case 4:
			fprintf(stderr,"Error : line %d Need ','.\n", line);
			break;
	
		/**** Error for values ****/
		case 5:
			fprintf(stderr,"Error : line %d Invalid address.\n",line);
			break;

		case 6:
			fprintf(stderr, "Error : line %d Invalid decimal number.\n", line);
			break;

		case 7:
			fprintf(stderr, "Error : line %d Invalid hexadecimal number.\n", line);
			break;

		/**** Error for symbol ****/
		case 8:
			fprintf(stderr,"Error : line %d Already exist symbol.\n", line);
			break;

		case 9:
			fprintf(stderr,"Error : line %d Doesn't exist symbol.\n", line);
			break;

		case 10:
			fprintf(stderr,"Error : line %d Symbol name must start with alphabet.\n", line);
			break;

		/**** Error for directive ****/
		case 11:
			fprintf(stderr, "Error : line %d START directive must be at first line.(except comment)\n", line);
			break;

		case 12:
			fprintf(stderr, "Error : line %d This directive needs a symbol.\n", line);
			break;

		case 13:
			fprintf(stderr, "Error : line %d Invalid BYTE value.\n", line);
			break;
		
			/**** Error for mnemonic ****/
		case 14:
			fprintf(stderr,"Error : line %d format 1 mnemonic doesn't need parameter.\n",line);
			break;

		case 15:
			fprintf(stderr,"Error : line %d format 2 mnemonic only have register(X,A,S,T) as parameter.\n", line);
			break;

		case 16:
			fprintf(stderr, "Error : line %d additional parameter must be X register.\n",line);
			break;

		case 17:
			fprintf(stderr,"Error : line %d This mnemonic doesn't use format 4\n", line);
			break;

		case 18:
			fprintf(stderr, "Error : line %d Unknown mnemonic.\n", line);
			break;

		/**** Error for loc ****/
		case 19:
			fprintf(stderr, "Error : assemble location is out of memory.\n");
			break;

		case 20:
			fprintf(stderr, "Error : line %d Invalid location. Out of format 3 displacement range.\n", line);
			break;
	}
}

void error_in_loader(int status, char* filename){
	switch(status){
		case 1:
			fprintf(stderr, "Error : %s doesn't exist.\n", filename);
			break;
		case 2:
			fprintf(stderr, "Error : There is no files.\n");
			break;
	}
}
