#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "addr.h"
#include "constant.h"
#include "error.h"
#include "file.h"
#include "loader.h"
#include "string_process.h"
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

	init_estab();
	/**** File open complete ****/
	while((tmp = strtok(NULL, " ")) != NULL){
		file_cnt++;
		if(file_cnt == 1){
			fp_arr = (FILE**)calloc(file_cnt, sizeof(FILE*));
			filename_arr = (char**)calloc(file_cnt, sizeof(char*));
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
		for(i = 0; i < file_cnt - 1; i++){
			fclose(fp_arr[i]);
			free(filename_arr[i]);
		}
		free(filename_arr[i]);
		return 14;
	}

	if(file_cnt == 0){
		error_in_loader(2, NULL);
		return 14;
	}

	/**** Pass 1 ****/
	printf("%d\n",file_cnt);
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
			
			printf("%s\n",str_line);
			
			switch(str_line[0]){
				case 'H':
					diff = obj_record_H(str_line, csect, csaddr, &prog_length);
					break;

				case 'D':
					obj_record_D(str_line, csect, csaddr, diff);
					break;

				case 'E':
					flag = 1;
					break;

				default:
					continue;
			}

			if(flag) break;
		}
		csaddr += prog_length;
	}
	print_estab();

	/**** Pass 2 ****/

	return 0;
}

// return length;
int obj_record_H(char *str, char *csect, int csaddr, int *length){
	int start;
	sscanf(str,"H%06s%06X%06X",csect,&start,length);
	delete_trailing_whitespace(csect);
	add_at_estab(csect, csect, csaddr, *length);
	return csaddr - start;
}

void obj_record_D(char *str, char *csect, int csaddr, int diff){
	int curr_ptr = 1;
	int str_length = strlen(str);
	int addr;
	char symbol[MAX_INSTRUCTION];
	while(curr_ptr < str_length){
		sscanf(str+curr_ptr,"%06s%06X",symbol,&addr);
		addr += diff;
		add_at_estab(csect, symbol, addr, -1);
		curr_ptr += 12;
	}
}
