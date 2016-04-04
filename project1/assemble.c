#include <stdlib.h>
#include <string.h>
#include "assemble.h"
#include "assemble_command.h"
#include "constant.h"
#include "error.h"
#include "opcode.h"
#include "string_process.h"
#include "symbol.h"



/* Description : open file
 * Return value : File pointer
 * if error occurs, *err represent error code
 */
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

/* Description : print file content
 * return value : if error, return error value
 *                if do well, return 0
 */
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

/* Description : assemble file
 * return value : error code
 */
int command_assemble(){
	FILE* fp;
	static char str_line[MAX_STR_LENGTH];
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
	int i;
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
				error_in_assemble(11, line*5);
//				fprintf(stderr, "Error : line %d START directive must be at first line.(except comment)\n", line*5);
				error++;
			}
		}
	
		/**** Update current location ****/
		curr_loc += commands[line-1].format;
	}
	if(curr_loc > 0xFFFFF){
		error_in_assemble(19 ,-1);
		//		fprintf(stderr, "Error : assemble location is out of memory.\n");
		error++;
	}
	if(error){
		for(i = 0; i<line; i++){
			if(!strcmp(commands[i].mnemonic,"BYTE"))
				free(commands[i].obj_byte);
		}
		free(commands);
		delete_at_tmp_symbol(0);
		return 13;
	}

	/**** Pass 2 ****/
	create_objectcode(commands, line, &error);
	if(error){
		for(i = 0; i<line; i++){
			if(!strcmp(commands[i].mnemonic,"BYTE"))
				free(commands[i].obj_byte);
		}
		free(commands);
		delete_at_tmp_symbol(0);
		return 13;
	}

	/**** Create .lst, .obj files ****/
	lst_filename = make_lst(commands, line, filename); 
	obj_filename = make_obj(commands, line, filename, start_loc, curr_loc);
	printf("\toutput file : [%s], [%s]\n",lst_filename,obj_filename);
	
	/**** Free memories ****/
	for(i = 0; i<line; i++){
		if(!strcmp(commands[i].mnemonic,"BYTE"))
			free(commands[i].obj_byte);
	}
	free(commands);
	free(lst_filename);
	free(obj_filename);
	
	/* Delete temporary constructed symbol table 
	 * Delete existing symbol table
	 * make new symbol table for command symbol
	 */
	delete_at_symbol_table();
	delete_at_tmp_symbol(1);
	return 0;
}



/**** Pass 2 ****/
/**** Create object code ****/
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
			/**** If mnemonic is LDB, set base register ****/
			if(!strcmp(commands[i].mnemonic,"LDB")){
				sym_tmp = find_at_symbol(commands[i].param1+1);
				if(!sym_tmp){
					error_in_assemble(9, (i+1)*5);
					//fprintf(stderr,"Error : line %d Doesn't exist symbol.\n",(i+1)*5);
					(*error)++;
				}
				else{
					b = sym_tmp->loc;
				}
			}	
			
			/**** set PC register ****/
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
						/**** Immediate addressing ****/
						case '#':
							/**** set ni = 01 ****/
							objcode |= 0x1;
							/**** format 4 ****/
							if(commands[i].format == 4){
								if((disp = is_decimal(commands[i].param1+1)) < 0){
									sym_tmp = find_at_symbol(commands[i].param1+1);
									if(!sym_tmp){
										error_in_assemble(9, (i+1)*5);
										//fprintf(stderr,"Error : line %d Doesn't exist symbol.\n",(i+1)*5);
										(*error)++;
									}
									else{
										disp = sym_tmp->loc;
									}
								}
							}
							/**** format 3 ****/
							else{
								/**** Find at symbol table ****/
								if((disp = is_decimal(commands[i].param1+1)) < 0){
									sym_tmp = find_at_symbol(commands[i].param1+1);
									if(!sym_tmp){
										error_in_assemble(9, (i+1)*5);
										//fprintf(stderr,"Error : line %d Doesn't exist symbol.\n",(i+1)*5);
										(*error)++;
									}
									else{
										loc_diff = sym_tmp->loc - pc;
										/**** use PC register relative addressing ****/
										if(loc_diff >= -2048 && loc_diff <= 2047){
											xbpe |= 0x2;
											disp = loc_diff;
										}
										/**** Out of PC register range ****/
										else{
											/**** Not using base register ****/
											if(b < 0){
												error_in_assemble(20, line*5);
												//fprintf(stderr, "Error : line %d Invalid location. Out of format 3 displacement range.\n", line*5);
												(*error)++;
											}
											/**** use base register relative addressing ****/
											else{
												loc_diff = sym_tmp->loc - b;
												if(loc_diff >= 0 && loc_diff <= 4095){
													xbpe |= 0x4;
													disp = loc_diff;
												}
												/**** out of base register range ****/
												else{
													error_in_assemble(20, line*5);
													//fprintf(stderr, "Error : line %d Invalid location. Out of format 3 displacement range.\n", line*5);
													(*error)++;
												}
											}
										}
									}
								}
								/**** for managing minus displacement ****/
								disp &= 0xFFF;
							}
							break;
						
						/**** Indirecting addressing ****/
						case '@':
							/**** set ni = 10 ****/
							objcode |= 0x2;
							/**** format 4 ****/
							if(commands[i].format == 4){
								if((disp = is_decimal(commands[i].param1+1)) < 0){
									sym_tmp = find_at_symbol(commands[i].param1+1);
									if(!sym_tmp){
										error_in_assemble(9, (i+1)*5);
										//fprintf(stderr,"Error : line %d Doesn't exist symbol.\n",(i+1)*5);
										(*error)++;
									}
									else{
										disp = sym_tmp->loc;
									}
								}
							}
							/**** format 3 ****/
							else{
								if((disp = is_decimal(commands[i].param1+1)) < 0){
									sym_tmp = find_at_symbol(commands[i].param1+1);
									if(!sym_tmp){
										error_in_assemble(9, (i+1)*5);
										//fprintf(stderr,"Error : line %d Doesn't exist symbol.\n",(i+1)*5);
										(*error)++;
									}
									else{
										loc_diff = sym_tmp->loc - pc;
										
										/**** PC relative addressing ****/
										if(loc_diff >= -2048 && loc_diff <= 2047){
											xbpe |= 0x2;
											disp = loc_diff;
										}

										/**** base relative addressing ****/
										else{
											if(b < 0){
												error_in_assemble(20, line*5);
												//fprintf(stderr, "Error : line %d Invalid location. Out of format 3 displacement range.\n", line*5);
												(*error)++;
											}
											else{
												loc_diff = sym_tmp->loc - b;
												if(loc_diff >= 0 && loc_diff <= 4095){
													xbpe |= 0x4;
													disp = loc_diff;
												}
												else{
													error_in_assemble(20, line*5);
													//fprintf(stderr, "Error : line %d Invalid location. Out of format 3 displacement range.\n", line*5);
													(*error)++;
												}
											}
										}
									}
								}
								disp &= 0xFFF;
							}
							break;

						/**** simple addressing ****/
						default :
							objcode |= 3;
							/**** format 4 ****/
							if(commands[i].format == 4){
								if((disp = is_decimal(commands[i].param1+1)) < 0){
									sym_tmp = find_at_symbol(commands[i].param1);
									if(!sym_tmp){
										error_in_assemble(9, (i+1)*5);
										//fprintf(stderr,"Error : line %d Doesn't exist symbol.\n",(i+1)*5);
										(*error)++;
									}
									else{
										disp = sym_tmp->loc;
									}
								}
							}

							/**** format 3 ****/
							else{
								if((disp = is_decimal(commands[i].param1+1)) < 0){
									sym_tmp = find_at_symbol(commands[i].param1);
									if(!sym_tmp){
										error_in_assemble(9, (i+1)*5);
										//fprintf(stderr,"Error : line %d Doesn't exist symbol.\n",(i+1)*5);
										(*error)++;
									}
									else{
										loc_diff = sym_tmp->loc - pc;
										
										/**** PC relative addressing ****/
										if(loc_diff >= -2048 && loc_diff <= 2047){
											xbpe |= 0x2;
											disp = loc_diff;
										}

										/**** base relative addressing ****/
										else{
											if(b < 0){
												error_in_assemble(20, line*5);
												//fprintf(stderr, "Error : line %d Invalid location. Out of format 3 displacement range.\n", line*5);
												(*error)++;
											}
											else{
												loc_diff = sym_tmp->loc - b;
												if(loc_diff >= 0 && loc_diff <= 4095){
													xbpe |= 0x4;
													disp = loc_diff;
												}
												else{
													error_in_assemble(20, line*5);
													//fprintf(stderr, "Error : line %d Invalid location. Out of format 3 displacement range.\n", line*5);
													(*error)++;
												}
											}
										}
									}
								}
								disp &= 0XFFF;
							}
							break;
					} // end switch case
				 	
					/**** make object code ****/
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
											 commands[i].obj_byte = (unsigned char*)calloc(len-3,sizeof(unsigned char));
											 for(j = 2; j < len-1; j++){
												 commands[i].obj_byte[j-2] = commands[i].param1[j];
											 }
											 commands[i].byte_len = len-3;
											 break;
										 }
						case 'X':{
											 int tmp;
											 commands[i].obj_byte = (unsigned char*)calloc((len-3)/2,sizeof(unsigned char));
											 for(j = 2; j < len-1; j+=2){
												 sscanf(commands[i].param1+j,"%02X",&tmp);
												 commands[i].obj_byte[(j-2)/2] = tmp;
											 }
											 commands[i].byte_len = (len-3)/2;
											 break;
										 }
					}
				}
			}
		}
	}
}


/**** make .lst file ****/
char* make_lst(assemble_table *commands, int line, char *filename){
	int i, j;
	FILE *fp;
	char *lst_filename = (char*)calloc(MAX_STR_LENGTH,sizeof(char));
	int file_len;
	int file_dot = 0;

	/**** create filename.lst file ****/
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

		/**** format 2 printing ****/
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

		/**** print object code ****/
		if(commands[i].opcode >= -1){
			if(strcmp(commands[i].mnemonic,"BYTE")){
				/**** print mnemonic object code ****/
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
				/**** directive WORD printing ****/
				else{
					fprintf(fp,"%06X",commands[i].objectcode);
				}
			}
			/**** directive BYTE printing ****/
			else{
				int byte_len = commands[i].byte_len;
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

/**** make .obj file ****/
char* make_obj(assemble_table* commands, int line, char* filename, int start, int end){
	int i, j;
	FILE *fp;
	char *obj_filename = (char*)calloc(MAX_STR_LENGTH,sizeof(char));
	int file_len;
	int file_dot = 0;
	int line_size;
	int curr_print_size;
	
	/**** make string that filename.obj ****/
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

	/**** header ****/
	fprintf(fp,"H%-6s%06X%06X\n",commands[i].symbol,start,end-start);
	i++;
	
	/**** print object code ****/
	while(i<line){
		curr_print_size = 0;

		/**** if opcode == -2, ignore ****/
		if(commands[i].opcode == -2){
			i++;
			continue;
		}
		
		/**** get size of a line ****/
		line_size = size_in_a_line(commands, i, line);
		if(line_size <= 0) continue;
		fprintf(fp,"T%06X%02X",commands[i].loc,line_size);
		
		for(;i<line; i++){
			/**** for memory allocation to RESB, RESW ****/
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
			
			/**** print object code ****/
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
				int byte_len = commands[i].byte_len;
				for(j = 0; j<byte_len; j++){
					fprintf(fp,"%02X",commands[i].obj_byte[j]);
				}
			}
		}
	}

	/**** print modification record ****/
	fprintf(fp,"\n");
	for(i = 0; i<line; i++){
		if(commands[i].mnemonic[0] == '+'){
			if(commands[i].param1[0] == '#' || commands[i].param1[0] == '@'){
				if(is_decimal(commands[i].param1+1) >= 0) continue;
			}
			else{
				if(is_decimal(commands[i].param1) >= 0) continue;
			}
			fprintf(fp,"M%06X%02X\n",commands[i].loc+1,commands[i].format+1);
		}
	}

	/**** end ****/
	fprintf(fp,"E%06X\n",start);
	fclose(fp);
	return obj_filename;
}

/**** get size in a line at .obj file ****/
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
