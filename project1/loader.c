#include <stdlib.h>
#include <string.h>

#include "addr.h"
#include "constant.h"
#include "debug.h"
#include "error.h"
#include "file.h"
#include "loader.h"
#include "memory.h"
#include "run.h"
#include "string_process.h"
#include "structures.h"
#include "symbol.h"

int command_loader(){
	FILE **fp_arr;
	
	char **filename_arr;
	char *tmp;
	char str_line[MAX_OBJ_LENGTH];
	char csect[MAX_INSTRUCTION];

	int error = 0;
	int file_cnt = 0;
	int progaddr = get_Addr();
	int csaddr = progaddr;
	int diff;
	int flag = 0;
	int prog_length;
	int str_length;
	int i, c;
	int *modif;

	init_estab();
	/**** File open complete ****/
	while((tmp = strtok(NULL, " ")) != NULL){
		file_cnt++;
		if(file_cnt == 1){
			fp_arr = (FILE**)calloc(file_cnt, sizeof(FILE*));
			filename_arr = (char**)calloc(file_cnt, sizeof(char*));
		}
		else{
			fp_arr = (FILE**)realloc(fp_arr, file_cnt * sizeof(FILE*));
			filename_arr = (char**)realloc(filename_arr, file_cnt * sizeof(char*));
		}
		filename_arr[file_cnt-1] = (char*)calloc(strlen(tmp),sizeof(char));
		strcpy(filename_arr[file_cnt-1], tmp);
		fp_arr[file_cnt-1] = open_file(&error, filename_arr[file_cnt-1], 0);

		if(error > 0) {
			error_in_loader(1, filename_arr[file_cnt-1]);
			break;
		}
	}
	
	if(error > 0){
		memfree_loader(file_cnt - 1, filename_arr, fp_arr);
		return 14;
	}

	if(file_cnt == 0){
		error_in_loader(2, NULL);
		return 14;
	}

	/**** Pass 1 ****/
	for(i = 0; i<file_cnt; i++){
		flag = 0;
		c = 0;
		while(c != EOF){
			str_length = 0;
			memset(str_line, 0, MAX_OBJ_LENGTH);
			
			/**** Read a line ****/
			while((c = fgetc(fp_arr[i])) != '\n' && c != EOF){
				if(str_length >= MAX_STR_LENGTH)
					continue;
				str_line[str_length++] = (char)c;
			}
			str_line[str_length] = '\0';
			
			switch(str_line[0]){
				case 'H':
					diff = obj_record_H(str_line, csect, csaddr, &prog_length, &error);
					break;

				case 'D':
					obj_record_D(str_line, csect, diff, &error);
					break;

				case 'E':
					flag = 1;
					break;

				default:
					continue;
			}

			if(error){	
				init_estab();
				memfree_loader(file_cnt, filename_arr, fp_arr);
				return 14;
			}
			
			if(flag) break;
		}
		csaddr += prog_length;
		fseek(fp_arr[i],0,SEEK_SET);
	}

	csaddr = progaddr;
	/**** Pass 2 ****/
	for(i = 0; i<file_cnt; i++){
		flag = 0;
		c = 0;
		while(c != EOF){
			str_length = 0;
			memset(str_line, 0, MAX_OBJ_LENGTH);

			/**** Read a line ****/
			while((c = fgetc(fp_arr[i])) != '\n' && c != EOF){
				if(str_length >= MAX_STR_LENGTH)
					continue;
				str_line[str_length++] = (char)c;
			}
			str_line[str_length] = '\0';

			switch(str_line[0]){
				case 'H':{
									 extsym_table *tmp;
									 int start;
									 
									 sscanf(str_line,"H%06s%06X",csect,&start);
									 delete_trailing_whitespace(csect);
									 diff = csaddr - start;
									 tmp = find_at_estab(csect);
									 prog_length = tmp->length;
									 modif = (int*)calloc(1,sizeof(int));
									 modif[0] = tmp->addr;
								 }
								 break;
				case 'R':
					obj_record_R(str_line, modif, &error);
					break;
				case 'T':
					obj_record_T(str_line, diff);
					break;
				case 'M':
					obj_record_M(str_line, modif, diff);
					break;
			}
			
			if(error){
				memfree_loader(file_cnt, filename_arr, fp_arr);
				free(modif);
				return 14;
			}
		}
		free(modif);
		csaddr += prog_length;
	}
	
	print_estab();
	set_start_addr(progaddr);
	set_end_addr(csaddr-1);
	init_reg();
	init_curr_bp();
	return 0;
}

// return length;
int obj_record_H(char *str, char *csect, int csaddr, int *length, int *error){
	int start;
	extsym_table *tmp;

	sscanf(str,"H%06s%06X%06X",csect,&start,length);
	delete_trailing_whitespace(csect);
	tmp = find_at_estab(csect);
	
	if(tmp != NULL){
		error_in_loader(3,csect);
		(*error)++;
		return -1;
	}
	add_at_estab(csect, csect, csaddr, *length);
	return csaddr - start;
}

void obj_record_D(char *str, char *csect, int diff, int *error){
	int curr_ptr = 1;
	int str_length = strlen(str);
	int addr;
	char symbol[MAX_INSTRUCTION];
	extsym_table *tmp;
	
	while(curr_ptr < str_length){
		sscanf(str+curr_ptr,"%06s%06X",symbol,&addr);
		delete_trailing_whitespace(symbol);
		
		tmp = find_at_estab(symbol);
		if(tmp != NULL){
			error_in_loader(3,symbol);
			(*error)++;
			error = 0;
		}
		
		addr += diff;
		add_at_estab(csect, symbol, addr, -1);
		curr_ptr += 12;
	}
}

void obj_record_R(char *str, int* modif, int* error){
	int curr_ptr = 1;
	int str_length = strlen(str);
	int index;
	char symbol[MAX_INSTRUCTION];
	extsym_table *tmp;

	while(curr_ptr < str_length){
		sscanf(str+curr_ptr, "%02X%06s",&index, symbol);
		delete_trailing_whitespace(symbol);
		
		modif = (int*)realloc(modif, index * sizeof(int));
		
		tmp = find_at_estab(symbol);
		if(tmp == NULL){
			(*error)++;
			error_in_loader(4,symbol);
			return;
		}
		
		modif[index-1] = tmp->addr;
		curr_ptr += 8;
	}
}

void obj_record_T(char *str, int diff){
	int curr_ptr = 1;
	int code_length;
	int start_addr;
	int curr_addr;
	int value;
	
	sscanf(str+curr_ptr,"%06X%02X",&start_addr,&code_length);

	code_length *= 2;
	curr_ptr += 8;
	curr_addr = start_addr + diff;
	while(curr_ptr < code_length + 9){
		sscanf(str+curr_ptr,"%02X",&value);
		set_memory(curr_addr, value);
		curr_addr++;
		curr_ptr += 2;
	}
}

void obj_record_M(char *str, int *modif, int diff){
	int addr;
	int size;
	int sign;
	int index;
	int get;
	int tmp;
	int mask;
	int value;
	char sign_char;

	sscanf(str,"M%06X%02X%c%02X",&addr, &size, &sign_char, &index);
	addr += diff;
	if(sign_char == '+')
		sign = 1;
	else sign = -1;

	if(size % 2)
		get = size/2 + 1;
	else
		get = size/2;
	
	/**** get value from memory (one byte at once) ****/
	tmp = addr;
	while(get > 0){
		value = value << 8;
		value += get_memory(tmp);
		tmp++;
		get--;
	}
	/**** Make mask 0x00...0FF....F ****/
	get = size;
	while(get > 0){
		mask = mask << 4;
		mask += 0xF;
		get--;
	}

	/**** Masking 0xFFF... to value ****/
	tmp = mask & value;
	
	tmp += modif[index-1]*sign;
	tmp = mask & tmp;

	/**** ~mask = 0xFF...F0...0 ****/
	value = value & (~mask);
	value += tmp;

	/**** edit at memory ****/
	tmp = addr;
	if(size % 2)
		get = size/2 + 1;
	else
		get = size/2;
	while(get > 0){
		set_memory(tmp, ((value >> ((get-1)*8)) & 0xFF));
		tmp++;
		get--;
	}
}

void memfree_loader(int tot, char** filename_arr, FILE **fp_arr){
	int i;

	for(i = 0; i < tot; i++){
		fclose(fp_arr[i]);
		free(filename_arr[i]);
	}
	free(fp_arr);
	free(filename_arr);
}
