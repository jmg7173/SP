#include <stdlib.h>
#include <string.h>
#include "file_processing.h"
#include "opcode.h"
#include "string_process.h"

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

static symbol_table *symbols;
static symbol_table *tmp_table = NULL;

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
	char *lst_filename, *obj_filename;
	int length;
	int error = 0;
	int c;
	int line = 0;
	int curr_loc;
	int first_command = 0;
	int start_loc;
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
		memset(str_line,0,MAX_STR_LENGTH);
		if(!strcmp(commands[line-1].mnemonic,"END")) break;
		/**** Read a line ****/
		while((c = fgetc(fp)) != '\n' && c != EOF){
			if(length >= MAX_STR_LENGTH)
				continue;
			str_line[length++] = (char)c;
		}
		str_line[length] = '\0';
		
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
			if(strcmp(commands[line-1].mnemonic, "START"))
				curr_loc = 0, start_loc = 0;
			else
				start_loc = curr_loc;
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
	if(curr_loc > 0xFFFFF){
		fprintf(stderr, "Error : assemble location is out of memory.\n");
		error++;
	}
	if(error){
		return 13;
	}

	create_objectcode(commands, line, &error);
	if(error){
		return 14;
	}
	lst_filename = make_lst(commands, line, filename); 
	obj_filename = make_obj(commands, line, filename, start_loc, curr_loc);
	printf("\toutput file : [%s], [%s]\n",lst_filename,obj_filename);
	return 0;
}

int command_symbol(){
	symbol_table *tmp = symbols;
	int cnt = 0;
	while(tmp != NULL){
		printf("\t%s\t%04X\n",tmp->symbol, tmp->loc);
		tmp = tmp->next;
		cnt++;
	}
	if(cnt == 0) return -1;
	return 0;
}

/**** Pass 1 ****/
assemble_table line_to_command(char* str, int* error, int* curr_loc, int line){
	/**** Usable registers
	 * X:1, A:0, S:4, T:5
	 */
	char *tmp;
	int i;
	int flag_directive = 0;
	int numtmp;
	int comma_checker;
	assemble_table new_table = {.symbol = {0}, .mnemonic = {0}, .loc = 0};
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
	 * opcode : -1 if object code needed else -2
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
		
		new_table.opcode = -2;
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
	if(!node_tmp){
			if(find_at_symbol(tmp)){
				fprintf(stderr,"Error : line %d Already exist symbol.\n", line*5);
				(*error)++;
			}
			if(tmp[0] >= '0' && tmp[0] <='9'){
				fprintf(stderr,"Error : line %d Symbol name must start with alphabet.\n", line*5);
				(*error)++;
			}
			else
				add_at_tmp_symbol(tmp, *curr_loc, line*5);
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
									new_table.opcode = -2;
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
								 */

								/* TODO : How to parsing numbers or ASCII inside ' '							 
								 *        How to manage errors! : Too much/less characters, 
								 *        										  	Doesn't exist X or C,
								 *        										  	Doesn't exist ' '
								 */
				case 4: {
									strcpy(new_table.param1, tmp+strlen(tmp)+1);
									numtmp = direc_byte_check(new_table.param1);
									if(numtmp <= 0){
										fprintf(stderr, "Error : line %d Invalid BYTE value.\n", line*5);
										(*error)++;
									}
									new_table.format = numtmp;
									new_table.opcode = -1;
									break;
								}
				case 5: {
									tmp = strtok(NULL, " ");

									comma_checker = comma_check(tmp);
									if(comma_checker){
										fprintf(stderr, "Error : line %d Additional parameter doesn't need.\n", line*5);
										(*error)++;
									}

									numtmp = is_decimal(tmp);
									if(numtmp < 0){
										fprintf(stderr, "Error : line %d Invalid decimal number.\n", line*5);
										(*error)++;
									}
									new_table.format = numtmp;
									new_table.opcode = -1;
									strcpy(new_table.param1, tmp);
									if((tmp = strtok(NULL, " "))){
										fprintf(stderr, "Error : line %d Too much commands.\n", line*5);
										(*error)++;
									}
									break;
								}
								/**** RESB, RESW ****/
								/* Symbol must exists.
								 * parameter : hexadecimal numbers.
								 */
				case 6: {
									tmp = strtok(NULL, " ");

									comma_checker = comma_check(tmp);
									if(comma_checker){
										fprintf(stderr, "Error : line %d Additional parameter doesn't need.\n", line*5);
										(*error)++;
									}

									numtmp = is_decimal(tmp);
									if(numtmp < 0){
										fprintf(stderr, "Error : line %d Invalid decimal number.\n", line*5);
										(*error)++;
									}
									new_table.format = numtmp;
									new_table.opcode = -2;
									strcpy(new_table.param1, tmp);
									if((tmp = strtok(NULL, " "))){
										fprintf(stderr, "Error : line %d Too much commands.\n", line*5);
										(*error)++;
									}
									break;
								}
				case 7: {
									tmp = strtok(NULL, " ");

									comma_checker = comma_check(tmp);
									if(comma_checker){
										fprintf(stderr, "Error : line %d Additional parameter doesn't need.\n", line*5);
										(*error)++;
									}

									numtmp = is_decimal(tmp) * 3;
									if(numtmp < 0){
										fprintf(stderr, "Error : line %d Invalid decimal number.\n", line*5);
										(*error)++;
									}
									new_table.format = numtmp;
									new_table.opcode = -2;
									strcpy(new_table.param1, tmp);
									if((tmp = strtok(NULL, " "))){
										fprintf(stderr, "Error : line %d Too much commands.\n", line*5);
										(*error)++;
									}
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
	symbol_table *tmp = tmp_table;
	int find = 0;
	while(tmp){
		if(!strcmp(tmp->symbol,str)){
			find = 1;
			break;
		}
		tmp = tmp->next;
	}
	if(find){
		return tmp;
	}
	return NULL;
}

void add_at_tmp_symbol(const char *str, int curr_loc, int line){
	symbol_table *new_node;
	symbol_table *tmp = tmp_table;
	if(tmp_table == NULL){
		new_node = (symbol_table*)malloc(sizeof(symbol_table));
		new_node->line = line;
		new_node->loc = curr_loc;
		strcpy(new_node->symbol, str);
		new_node->next = NULL;
		tmp_table = new_node;
	}
	else{
		new_node = (symbol_table*)malloc(sizeof(symbol_table));
		while(tmp->next != NULL){
			tmp = tmp->next;
		}
		new_node->line = line;
		new_node->loc = curr_loc;
		strcpy(new_node->symbol, str);
		new_node->next = NULL;
		tmp->next = new_node;
	}
}

void create_objectcode(assemble_table *commands, int line, int *error){
	int i, j;
	int pc, b = -1;
	int objcode = 0;
	int disp;
	int loc_diff;
	int xbpe;
	//int x;	
	symbol_table *sym_tmp;
	
	for(i = 0; i<line; i++){
		if(commands[i].opcode >= -1){
			objcode = 0;
			if(!strcmp(commands[i].mnemonic,"LDB")){
				sym_tmp = find_at_symbol(commands[i].param1+1);
				if(!sym_tmp){
					fprintf(stderr,"Error : line %d Doesn't exist symbol.1\n",(i+1)*5);
					(*error)++;
				}
				else{
					b = sym_tmp->loc;
				}
			}	
			
			j = i+1;
			while(j<line){
				if(commands[j].opcode != -2){
					pc = commands[j].loc;
					break;
				}
				j++;
			}

			/**** object code for mnemonic ****/
			if(commands[i].opcode >= 0){
				xbpe = 0;
				disp = 0;
				loc_diff = 0;

				objcode += commands[i].opcode;
				/**** for format 3, 4 ****/
				if(commands[i].format == 3 || commands[i].format == 4){
					if(commands[i].format == 4)
						xbpe |= 0x1;

					if(commands[i].use_X){
						commands[i].param1[strlen(commands[i].param1)-1] = '\0';
						xbpe |= 0x8;
					}

					switch(commands[i].param1[0]){
						case '#':
							objcode |= 0x1;
							if(commands[i].format == 4){
								if((disp = is_decimal(commands[i].param1+1)) < 0){
									sym_tmp = find_at_symbol(commands[i].param1+1);
									if(!sym_tmp){
										fprintf(stderr,"Error : line %d Doesn't exist symbol.2\n",(i+1)*5);
										(*error)++;
									}
									else{
										disp = sym_tmp->loc;
									}
								}
								disp &= 0xFFFFF;
							}
							else{
								if((disp = is_decimal(commands[i].param1+1)) < 0){
									sym_tmp = find_at_symbol(commands[i].param1+1);
									if(!sym_tmp){
										fprintf(stderr,"Error : line %d Doesn't exist symbol.3\n",(i+1)*5);
										(*error)++;
									}
									else{
										loc_diff = sym_tmp->loc - pc;
										if(loc_diff >= -4096 && loc_diff <= 4095){
											xbpe |= 0x2;
											disp = loc_diff;
										}
										else{
											if(b < 0){
												fprintf(stderr, "Error : line %d Invalid location. Out of format 3 displacement range.\n", line*5);
												(*error)++;
											}
											else{
												loc_diff = sym_tmp->loc - b;
												if(loc_diff >= -4096 && loc_diff <= 4095){
													xbpe |= 0x4;
													disp = loc_diff;
												}
												else{
													fprintf(stderr, "Error : line %d Invalid location. Out of format 3 displacement range.\n", line*5);
													(*error)++;
												}
											}
										}
									}
								}
								disp &= 0xFFF;
							}
							break;
						case '@':
							objcode |= 0x2;
							if(commands[i].format == 4){
								if((disp = is_decimal(commands[i].param1+1)) < 0){
									sym_tmp = find_at_symbol(commands[i].param1+1);
									if(!sym_tmp){
										fprintf(stderr,"Error : line %d Doesn't exist symbol.4\n",(i+1)*5);
										(*error)++;
									}
									else{
										disp = sym_tmp->loc;
									}
								}
								disp &= 0xFFFFF;
							}
							else{
								if((disp = is_decimal(commands[i].param1+1)) < 0){
									sym_tmp = find_at_symbol(commands[i].param1+1);
									if(!sym_tmp){
										fprintf(stderr,"Error : line %d Doesn't exist symbol.5\n",(i+1)*5);
										(*error)++;
									}
									else{
										loc_diff = sym_tmp->loc - pc;
										if(loc_diff >= -2048 && loc_diff <= 2047){
											xbpe |= 0x2;
											disp = loc_diff;
										}
										else{
											if(b < 0){
												fprintf(stderr, "Error : line %d Invalid location. Out of format 3 displacement range.\n", line*5);
												(*error)++;
											}
											else{
												loc_diff = sym_tmp->loc - b;
												if(loc_diff >= 0 && loc_diff <= 4095){
													xbpe |= 0x4;
													disp = loc_diff;
												}
												else{
													fprintf(stderr, "Error : line %d Invalid location. Out of format 3 displacement range.\n", line*5);
													(*error)++;
												}
											}
										}
									}
								}
								disp &= 0xFFF;
							}
							break;
						default :
							objcode |= 3; //simple addressing
							if(commands[i].format == 4){
								if((disp = is_decimal(commands[i].param1+1)) < 0){
									sym_tmp = find_at_symbol(commands[i].param1);
									if(!sym_tmp){
										fprintf(stderr,"Error : line %d Doesn't exist symbol.6\n",(i+1)*5);
										(*error)++;
									}
									else{
										disp = sym_tmp->loc;
									}
								}
								disp &= 0xFFFFF;
							}
							else{
								if((disp = is_decimal(commands[i].param1+1)) < 0){
									sym_tmp = find_at_symbol(commands[i].param1);
									if(!sym_tmp){
										fprintf(stderr,"Error : line %d Doesn't exist symbol.7\n",(i+1)*5);
										(*error)++;
									}
									else{
										loc_diff = sym_tmp->loc - pc;
										if(loc_diff >= -4096 && loc_diff <= 4095){
											xbpe |= 0x2;
											disp = loc_diff;
										}
										else{
											if(b < 0){
												fprintf(stderr, "Error : line %d Invalid location. Out of format 3 displacement range.\n", line*5);
												(*error)++;
											}
											else{
												loc_diff = sym_tmp->loc - b;
												if(loc_diff >= -4096 && loc_diff <= 4095){
													xbpe |= 0x4;
													disp = loc_diff;
												}
												else{
													fprintf(stderr, "Error : line %d Invalid location. Out of format 3 displacement range.\n", line*5);
													(*error)++;
												}
											}
										}
									}
								}
								disp &= 0XFFF;
							}
							break;
					}
				 	objcode = objcode << 4;
					objcode |= xbpe;
					if(commands[i].format == 4)
						objcode = objcode << 20;
					else
						objcode = objcode << 12;
					objcode |= disp;
				}

				/**** for format 1 ****/
				else if(commands[i].format == 1){
					objcode = commands[i].opcode;
				}
				/**** for format 2 ****/
				else if (commands[i].format == 2){
					objcode = commands[i].opcode;
					objcode = objcode << 4;
					objcode |= bit_reg(commands[i].reg1);
					objcode = objcode << 4;
					objcode |= bit_reg(commands[i].reg2);
				}
				commands[i].objectcode = objcode;
			}

			/**** object code for memory(BYTE, WORD) ****/
			else{
				/**** WORD ****/
				if(!strcmp(commands[i].mnemonic,"WORD")){
					commands[i].objectcode = is_decimal(commands[i].param1);
				}
				
				/**** BYTE ****/
				else{
					int len = strlen(commands[i].param1);
					switch(commands[i].param1[0]){
						case 'C':{
											 commands[i].obj_byte = (char*)calloc(len-3,sizeof(int));
											 for(j = 2; j < len-1; j++){
												 commands[i].obj_byte[j-2] = commands[i].param1[j];
											 }
											 break;
										 }
						case 'X':{
											 int tmp;
											 commands[i].obj_byte = (char*)calloc((len-3)/2,sizeof(int));
											 for(j = 2; j < len-1; j+=2){
												 sscanf(commands[i].param1+j,"%02X",&tmp);
												 commands[i].obj_byte[(j-2)/2] = tmp;
											 }
											 break;
										 }
					}
				}
			}
		}
	}
}

int bit_reg(char reg){
	switch(reg){
		case 'A':
			return 0;
		case 'X':
			return 1;
		case 'S':
			return 4;
		case 'T':
			return 5;
		case '\0':
			return 0;
	}
	return -1;
}

char* make_lst(assemble_table *commands, int line, char *filename){
	int i, j;
	FILE *fp;
	char *lst_filename = (char*)calloc(MAX_STR_LENGTH,sizeof(char));
	int file_len;
	int file_dot = 0;

	strcpy(lst_filename, filename);
	file_len = strlen(lst_filename);
	for(i = file_len - 1; i >= 0; i--){
		if(lst_filename[i] == '.'){
			file_dot = i;
			break;
		}
	}
	if(file_dot){
		lst_filename[file_dot+1] = '\0';
		strcat(lst_filename,"lst");
	}
	else
		strcat(lst_filename, ".lst");

	fp = fopen(lst_filename, "w");

	for(i = 0; i<line; i++){
		/**** line number ****/
		fprintf(fp,"%d\t",commands[i].line);
		
		/**** location ****/
		/* Don't print loc when mnemonic is directive
		 * exception : BYTE, WORD -> opcode set as -1
		 */
		if(!strcmp(commands[i].mnemonic,"START") ||
				!strcmp(commands[i].mnemonic,"RESB") ||
				!strcmp(commands[i].mnemonic,"RESW")
				)
			fprintf(fp,"%04X\t",commands[i].loc);
		else if(commands[i].opcode < -1)
			fprintf(fp, "\t");
		else
			fprintf(fp,"%04X\t",commands[i].loc);

		/**** symbol printing ****/
		/* print symbol
		 * exception : comment -> print at symbol
		 */
		if(commands[i].mnemonic[0] == '.'){
			fprintf(fp,".\t%s\n",commands[i].param1);
			continue;
		}
		fprintf(fp,"%s\t%s\t",
				commands[i].symbol,
				commands[i].mnemonic
				);
		if(commands[i].format == 2){
			if(commands[i].reg2 != '\0')
				fprintf(fp,"%c, %c\t\t\t",commands[i].reg1,commands[i].reg2);
			else
				fprintf(fp,"%c\t\t\t",commands[i].reg1);
		}
		else{
			fprintf(fp,"%s",commands[i].param1);
			if(commands[i].use_X)
				fprintf(fp,", X\t\t");
			else
				fprintf(fp,"\t\t\t");
		}
		if(commands[i].opcode >= -1){
			if(strcmp(commands[i].mnemonic,"BYTE")){
				if(strcmp(commands[i].mnemonic,"WORD")){
					switch(commands[i].format){
						case 1:
							fprintf(fp,"%02X",commands[i].objectcode);
							break;
						case 2:
							fprintf(fp,"%04X",commands[i].objectcode);
							break;
						case 3:
							fprintf(fp,"%06X",commands[i].objectcode);
							break;
						case 4:
							fprintf(fp,"%08X",commands[i].objectcode);
							break;
					}
				}
				else{
					fprintf(fp,"%06X",commands[i].objectcode);
				}
			}
			else{
				int byte_len = strlen(commands[i].obj_byte);
				for(j = 0; j<byte_len; j++){
					fprintf(fp,"%02X",commands[i].obj_byte[j]);
				}
			}
		}
		fprintf(fp,"\n");
	}
	fclose(fp);
	return lst_filename;
}

char* make_obj(assemble_table* commands, int line, char* filename, int start, int end){
	int i, j;
	FILE *fp;
	char *obj_filename = (char*)calloc(MAX_STR_LENGTH,sizeof(char));
	int file_len;
	int file_dot = 0;
	int line_size;
	int curr_print_size;
	
	strcpy(obj_filename, filename);
	file_len = strlen(obj_filename);
	for(i = file_len - 1; i >= 0; i--){
		if(obj_filename[i] == '.'){
			file_dot = i;
			break;
		}
	}
	if(file_dot){
		obj_filename[file_dot+1] = '\0';
		strcat(obj_filename,"obj");
	}
	else
		strcat(obj_filename, ".obj");

	fp = fopen(obj_filename, "w");
	
	i = 0;
	while(strcmp(commands[i].mnemonic,"START")) i++;

	fprintf(fp,"H%-6s%06X%06X\n",commands[i].symbol,start,end-start);
	i++;
	
	while(i<line){
		curr_print_size = 0;
		if(commands[i].opcode == -2){
			i++;
			continue;
		}
		
		line_size = size_in_a_line(commands, i, line);
		if(line_size <= 0) continue;
		fprintf(fp,"T%06X%02X",commands[i].loc,line_size);
		
		for(;i<line; i++){
			if(!strcmp(commands[i].mnemonic,"RESB") ||
					!strcmp(commands[i].mnemonic,"RESW")){
				fprintf(fp,"\n");
				break;
			}

			if(commands[i].opcode == -2) continue;
			
			curr_print_size += commands[i].format;
			
			if(curr_print_size > line_size){
				fprintf(fp,"\n");
				break;
			}
			
			if(strcmp(commands[i].mnemonic,"BYTE")){
				if(strcmp(commands[i].mnemonic,"WORD")){
					switch(commands[i].format){
						case 1:
							fprintf(fp,"%02X",commands[i].objectcode);
							break;
						case 2:
							fprintf(fp,"%04X",commands[i].objectcode);
							break;
						case 3:
							fprintf(fp,"%06X",commands[i].objectcode);
							break;
						case 4:
							fprintf(fp,"%08X",commands[i].objectcode);
							break;
					}
				}
				else{
					fprintf(fp,"%06X",commands[i].objectcode);
				}
			}
			else{
				int byte_len = strlen(commands[i].obj_byte);
				for(j = 0; j<byte_len; j++){
					fprintf(fp,"%02X",commands[i].obj_byte[j]);
				}
			}
		}
	}
	fprintf(fp,"\nE%06X\n",start);
	fclose(fp);
	return obj_filename;
}

int size_in_a_line(assemble_table* commands, int start, int line){
	int i;
	int size = 0;
	for(i = start; i<line; i++){
		if(!strcmp(commands[i].mnemonic,"RESB") ||
				!strcmp(commands[i].mnemonic,"RESW")){
			break;
		}

		if(commands[i].opcode == -2) continue;

		size += commands[i].format;
		
		if(size > 30){
			size -= commands[i].format;
			break;
		}
	}
	return size;
}
