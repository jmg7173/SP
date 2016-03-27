#include <stdlib.h>
#include <string.h>
#include "file_processing.h"
#include "opcode.h"
#include "string_process.h"

static symbol_table *symbols;
symbol_table *symbol_head = NULL;

FILE* open_file(int *err, char* filename){
	FILE *fp;
	char *tmp;
	
	fp = fopen(filename, "r");
	if(fp == NULL){
		*err = 12;
		return NULL;
	}
	if((tmp = strtok(NULL, " "))){
		fclose(fp);
		*err =  6;
		return NULL;
	}

	return fp;
}

int command_type(){
	FILE *fp;
	int c, error = 0;
	char *tmp;
	char filename[MAX_STR_LENGTH];

	tmp = strtok(NULL, " ");

	strcpy(filename, tmp);
	fp = open_file(&error, filename);
	if(error > 0)
		return error;

	while((c = fgetc(fp)) != EOF)
		fputc(c,stdout);
	
	fclose(fp);
	return 0;
}

int command_assemble(){
	FILE* fp;
	char str_line[MAX_STR_LENGTH];
	char *tmp;
	char filename[MAX_STR_LENGTH];
	int length;
	int error = 0;
	int c;
	int line = 0;
	int curr_loc;
	int first_command = 0;
	assemble_table *commands;

	tmp = strtok(NULL, " ");

	strcpy(filename, tmp);
	fp = open_file(&error, filename);
	if(error > 0)
		return error;
	
	/**** Pass1 ****/
	commands = (assemble_table*)calloc(1,sizeof(assemble_table));
	while(!feof(fp)){
		length = 0;
		if(!strcmp(commands[line-1].mnemonic,"END")) break;
		/**** Read a line ****/
		while((c = fgetc(fp)) != '\n' && c != EOF){
			if(length >= MAX_STR_LENGTH)
				continue;
			str_line[length++] = (char)c;
		}
		str_line[length] = '\0';
		printf("%s\n",str_line);
		delete_trailing_whitespace(str_line);
		line++;
		if(line != 1)
			commands = (assemble_table*)realloc(commands,sizeof(assemble_table)*line);
		
		/**** Add at commands ****/
		commands[line-1] = line_to_command(str_line, &error, &curr_loc, line);
		
		/* At first line, check if it is START 
		 * If it is START, curr_loc = START's parameter
		 * else, curr_loc = 0
		 */
		if(commands[0].mnemonic[0] != '.') first_command++;
		
		if(first_command == 1){
			if(strcmp(commands[0].mnemonic, "START"))
				curr_loc = 0;
		}

		if(first_command > 1){
			if(!strcmp(commands[line-1].mnemonic, "START")){
				fprintf(stderr, "Error : line %d START directive must be at first line.(except comment)\n", line*5);
				error++;
			}
		}
	
		/**** Update current location ****/
		curr_loc += commands[line-1].format;
	}
	if(error){
		return 13;
	}
	return 0;
}

int command_symbol(){
	symbol_table *tmp = symbol_head;
	int cnt = 0;
	while(tmp != NULL){
		printf("\t%s\t%04X\n",tmp->symbol, tmp->loc);
		tmp = tmp->next;
		cnt++;
	}
	if(cnt == 0) return -1;
	return 0;
}

assemble_table line_to_command(char* str, int* error, int* curr_loc, int line){
	/**** Usable registers
	 * X, A, S, T
	 */
	static const char *directives[DIRECTIVE_NUM] = {
		"END",
		"BASE",
		"NOBASE",
		"START",
		"BYTE",
		"WORD",
		"RESB",
		"RESW"
	};
	char *tmp;
	int i;
	int flag_directive = 0;
	int numtmp;
	int comma_checker;
	assemble_table new_table;
	hash_node* node_tmp = NULL;

	tmp = strtok(str," "); // First String
	
	/**** No Symbol commands ****/
	
	/**** Comment ****/
	/* Setting 
	 * symbol : NO
	 * mnemonic : .
	 * loc : -1
	 * line : line*5
	 * opcode : -2
	 * format : 0
	 * param1 : comments
	 * use_X : 0
	 * function end
	 */
	if(!strcmp(tmp,".")){
		strcpy(new_table.mnemonic,tmp);
		strcpy(new_table.param1,str+2);
		new_table.opcode = -2;
		new_table.format = 0;
		new_table.loc = -1;
		new_table.line = line*5;
		return new_table;
	}

	/**** Is Directive? ****/
	for(i = 0; i<DIRECTIVE_NUM; i++){
		if(!strcmp(tmp,directives[i])) {
			flag_directive = 1;
			break;
		}
	}

	/**** Directives ****/
	/* Setting 
	 * symbol : NO
	 * mnemonic : directive(END, BASE, NOBASE) else -> Error
	 * loc : -1
	 * line : line*5
	 * opcode : -1
	 * format : 0
	 * param1 : numbers
	 * use_X : 0
	 * function end
	 */
	if(flag_directive == 1){
		strcpy(new_table.mnemonic,tmp);
		if(i > 3){
			fprintf(stderr, "Error : line %d This directive needs a symbol.\n", line*5);
			(*error)++;
		}

		tmp = strtok(NULL, " ");
		strcpy(new_table.param1,tmp);
		
		new_table.opcode = -1;
		new_table.format = 0;
		new_table.loc = -1;
		new_table.line = line*5;
		
		if((tmp = strtok(NULL, " "))) {
			fprintf(stderr,"Error : line %d Too much parameter.\n",line*5);
			(*error)++;
		}
		if(!strcmp(new_table.mnemonic,"START")){
			numtmp = is_hexa(new_table.param1,1);
			if(numtmp < 0){
				fprintf(stderr,"Error : line %d Invalid address.\n",line*5);
				(*error)++;
				*curr_loc = 0;
			}
			else if(comma_check(new_table.param1)){
				fprintf(stderr,"Error : line %d Additional parameter doesn't need.\n", line*5);
				(*error)++;
				*curr_loc = numtmp;
			}
			else
				*curr_loc = numtmp;
			new_table.loc = *curr_loc;
		}
		return new_table;
	}

	/**** Is mnemonic? ****/	
	else{
		if(tmp[0] == '+')
			node_tmp = search_mnemonic(tmp+1);
		else
			node_tmp = search_mnemonic(tmp);
	}
	
	/**** A line with Symbol existing ****/
	// TODO : symbol table check!!!
	if(!node_tmp){
	/*	if(!find_at_symbol(tmp)){
			fprintf(stderr,"Error : line %d Already exist symbol.\n", line*5);
			(*error)++;
		}
		if(tmp[0] >= '0' && tmp[0] <='9'){
			fprintf(stderr,"Error : line %d Symbol name must start with alphabet.\n", line*5);
			(*error)++;
		}*/
		strcpy(new_table.symbol,tmp);
	}

	/**** Mnemonic that symbol doesn't exist ****/
	/**** Mnemonic ****/
	/* Setting 
	 * symbol : NO
	 * mnemonic : existing mnemonic
	 * loc : curr_loc
	 * line : line*5
	 * opcode : mnemonic opcode
	 * format : mnemonic format
	 * param1 : parameter
	 * use_X : use
	 * function end
	 */

	else{
		/**** Format 4 Check ****/
		/**** format setting ****/
		if(tmp[0] == '+'){
			strcpy(new_table.mnemonic, tmp + 1);
			if(node_tmp->format2 == 4)
				new_table.format = 4;
			else{
				fprintf(stderr,"Error : line %d This mnemonic doesn't use format 4\n",line * 5);
				(*error)++;
			}
		}
		else{
			strcpy(new_table.mnemonic, tmp);
			new_table.format = node_tmp->format1;
		}
		/**** opcode setting ****/
		new_table.opcode = node_tmp->opcode;
		new_table.loc = *curr_loc;
		new_table.line = line*5;

		/**** First parameter ****/
		/**** If format is 2, check if it is register ****/
		tmp = strtok(NULL, " ");
		if(!tmp){
			return new_table;
		}
		if(new_table.format == 1){
			if(tmp){
				fprintf(stderr,"Error : line %d format 1 mnemonic doesn't need parameter.\n",line*5);
				(*error)++;
			}
		}
		/**** parameter 1 setting ****/
		strcpy(new_table.param1,tmp);
		
		/**** reg1 setting ****/
		if(new_table.format == 2){
			if(strlen(tmp) > 2){
				fprintf(stderr,"Error : line %d format 2 mnemonic only have register(X,A,S,T) as parameter1.\n", line*5);
				(*error)++;
			}
			else{
				if(strlen(tmp) == 2){
					if(tmp[1] == ',' && tmp[0] != 'X' && tmp[0] != 'A' && tmp[0] != 'S' && tmp[0] != 'T'){
						fprintf(stderr,"Error : line %d format 2 mnemonic only have register(X,A,S,T) as parameter2.\n", line*5);
						(*error)++;
					}
					else new_table.reg1 = tmp[0];
				}
				else if(strcmp(tmp,"X") && strcmp(tmp,"A") && strcmp(tmp,"S") && strcmp(tmp,"T")){
						fprintf(stderr,"Error : line %d format 2 mnemonic only have register(X,A,S,T) as parameter3.\n", line*5);
						(*error)++;
					}
				else
					new_table.reg1 = tmp[0];
			}
		}

		/**** Have to Check
		 * Is it have comma?
		 *    - Then, it must have additional parameter
		 *    - If format 3 or 4, additional parameter must be register X
		 *    - If format 2, additional parameter must be registers
		 *    - If format 1, it must be Error
		 */
		if(comma_check(tmp)){
			if((tmp = strtok(NULL, " "))){
				if(comma_check(tmp)){
					fprintf(stderr, "Error : line  %d Too much parameter or comma.\n",line*5);
					(*error)++;
				}
				/**** use_X setting ****/
				else if((new_table.format == 4 || new_table.format == 3)){
					if(strcmp(tmp,"X")){
						fprintf(stderr, "Error : line %d additional parameter must be X register.\n",line*5);
						(*error)++;
					}
					else
						new_table.use_X = 1;
				}
				/**** reg2 setting ****/
				else if(new_table.format == 2){
					if(strlen(tmp) != 1){
						fprintf(stderr,"Error : line %d format 2 mnemonic only have register(X,A,S,T) as parameter4.\n", line*5);
						(*error)++;
					}
					else if(strcmp(tmp,"X") && strcmp(tmp,"A") && strcmp(tmp,"S") && strcmp(tmp,"T")){
						fprintf(stderr,"Error : line %d format 2 mnemonic only have register(X,A,S,T) as parameter5.\n", line*5);
						(*error)++;
					}
					else
						new_table.reg2 = tmp[0];
				}
			}
		}

		new_table.loc = *curr_loc;
		new_table.line = line*5;
		return new_table;
	}

	/**** Commands that have symbol ****/
	tmp = strtok(NULL, " ");
	for(i = 3; i<DIRECTIVE_NUM; i++){
		/* Setting
		 * symbol : already exists
		 * mnemonic : directive(START, BYTE, WORD, RESB, RESW)
		 * loc : curr_loc(PC)
		 * line : line*5
		 * opcode : BYTE, WORD - parameter(as hexa)
		 * 					START, RESB, RESW - -1
		 * format : START - 0, others - size relative.
		 * param1 : START, RESB, RESW - number
		 *          BYTE, WORD - C' ', X' '
		 * use_X : 0 <- Not touching
		 * function end
		 */
		if(!strcmp(tmp, directives[i])){
			/* Common :
			 * mnemonic, line, loc
			 */
			strcpy(new_table.mnemonic, tmp);
			switch(i){
				/**** START ****/
				// TODO : START directive must have symbol???
				case 3: {
									/* format, param1 */
									tmp = strtok(NULL, " ");

									/* Have to Check : 
									 * have comma? If so, print error.
									 * is hexadecimaL? If it isn't, print error.
									 * hexa is in range? If it isn't print error.
									 */
									comma_checker = comma_check(tmp);
									if(comma_checker){
										fprintf(stderr, "Error : line %d Additional parameter doesn't need.\n", line*5);
										(*error)++;
									}

									numtmp = is_hexa(tmp,1);
									if(numtmp < 0){
										fprintf(stderr, "Error : line %d Invalid hexadecimal number.\n", line*5);
										(*error)++;
									}
									*curr_loc = numtmp;
									new_table.format = 0;
									new_table.opcode = -1;
									strcpy(new_table.param1, tmp);
									if((tmp = strtok(NULL, " "))){
										fprintf(stderr, "Error : line %d Too much commands.\n", line*5);
										(*error)++;
									}
									break;
								}
								/**** BYTE, WORD ****/
								/* Symbol must exists.
								 * BYTE : X'  '  <- up to 60, only odd characters as hexa
								 *     or C'  '  <- up to 30 characters as ASCII
								 * WORD : number <- up to ?? characters as decimal
								 *     or C'  '  <- only 3 characters as ASCII
								 */

								/* TODO : How to parsing numbers or ASCII inside ' '							 
								 *        How to manage errors! : Too much/less characters, 
								 *        										  	Doesn't exist X or C,
								 *        										  	Doesn't exist ' '
								 */
				case 4: {
									break;
								}
				case 5: {
									break;
								}
								/**** RESB, RESW ****/
								/* Symbol must exists.
								 * parameter : hexadecimal numbers.
								 */
				case 6: {
									tmp = strtok(NULL, " ");
									comma_checker = comma_check(tmp);
									break;
								}
				case 7: {
									break;
								}
			}
			new_table.line = line*5;
			new_table.loc = *curr_loc;

			return new_table;
		}
	}
	/**** Is mnemonic? ****/	
	if(tmp[0] == '+')
		node_tmp = search_mnemonic(tmp+1);
	else
		node_tmp = search_mnemonic(tmp);

	/**** A line with Symbol existing ****/
	if(!node_tmp){
		printf("%s\n",tmp);
		fprintf(stderr, "Error : line %d Unknown mnemonic.\n", line*5);
		(*error)++;
	}

	/**** Mnemonic that symbol doesn't exist ****/
	/**** Mnemonic ****/
	/* Setting 
	 * symbol : exist
	 * mnemonic : existing mnemonic
	 * loc : curr_loc
	 * line : line*5
	 * opcode : mnemonic opcode
	 * format : mnemonic format
	 * param1 : parameter
	 * use_X : use
	 * function end
	 */

	/**** Format 4 Check ****/
	/**** format setting ****/
	if(tmp[0] == '+'){
		strcpy(new_table.mnemonic, tmp + 1);
		if(node_tmp->format2 == 4)
			new_table.format = 4;
		else{
			fprintf(stderr,"Error : line %d This mnemonic doesn't use format 4\n",line * 5);
			(*error)++;
		}
	}
	else{
		strcpy(new_table.mnemonic, tmp);
		new_table.format = node_tmp->format1;
	}
	/**** opcode setting ****/
	new_table.opcode = node_tmp->opcode;

	/**** First parameter ****/
	/**** If format is 2, check if it is register ****/
	tmp = strtok(NULL, " "); 
	if(new_table.format == 1){
		if(tmp){
			fprintf(stderr,"Error : line %d format 1 mnemonic doesn't need parameter.\n",line*5);
			(*error)++;
		}
	}
	/**** parameter 1 setting ****/
	strcpy(new_table.param1,tmp);

	/**** reg1 setting ****/
	if(new_table.format == 2){
		if(strlen(tmp) > 2){
			fprintf(stderr,"Error : line %d format 2 mnemonic only have register(X,A,S,T) as parameter6.\n", line*5);
			(*error)++;
		}
		else{
			if(strlen(tmp) == 2){
				if(tmp[1] == ',' && tmp[0] != 'X' && tmp[0] != 'A' && tmp[0] != 'S' && tmp[0] != 'T'){
					fprintf(stderr,"Error : line %d format 2 mnemonic only have register(X,A,S,T) as parameter7.\n", line*5);
					(*error)++;
				}
				else new_table.reg1 = tmp[0];
			}
			else if(strcmp(tmp,"X") && strcmp(tmp,"A") && strcmp(tmp,"S") && strcmp(tmp,"T")){
				fprintf(stderr,"Error : line %d format 2 mnemonic only have register(X,A,S,T) as parameter8.\n", line*5);
				(*error)++;
			}
			else
				new_table.reg1 = tmp[0];
		}
	}

	/**** Have to Check
	 * Is it have comma?
	 *    - Then, it must have additional parameter
	 *    - If format 3 or 4, additional parameter must be register X
	 *    - If format 2, additional parameter must be registers
	 *    - If format 1, it must be Error
	 */
	if(comma_check(tmp)){
		if((tmp = strtok(NULL, " "))){
			if(comma_check(tmp)){
				fprintf(stderr, "Error : line  %d Too much parameter or comma.\n",line*5);
				(*error)++;
			}
			/**** use_X setting ****/
			else if((new_table.format == 4 || new_table.format == 3)){
				if(strcmp(tmp,"X")){
					fprintf(stderr, "Error : line %d additional parameter must be X register.\n",line*5);
					(*error)++;
				}
				else
					new_table.use_X = 1;
			}
			/**** reg2 setting ****/
			else if(new_table.format == 2){
				if(strlen(tmp) != 1){
					fprintf(stderr,"Error : line %d format 2 mnemonic only have register(X,A,S,T) as parameter9.\n", line*5);
					(*error)++;
				}
				else if(strcmp(tmp,"X") && strcmp(tmp,"A") && strcmp(tmp,"S") && strcmp(tmp,"T")){
					fprintf(stderr,"Error : line %d format 2 mnemonic only have register(X,A,S,T) as parameter10.\n", line*5);
					(*error)++;
				}
				else
					new_table.reg2 = tmp[0];
			}
		}
	}

	new_table.loc = *curr_loc;
	new_table.line = line*5;
	return new_table;

}

symbol_table* find_at_symbol(const char *str){



}

void add_at_symbol(const char *str, int curr_loc, int line){


}
