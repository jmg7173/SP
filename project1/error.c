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
		default:
						break;
	}
	return 1;
}
