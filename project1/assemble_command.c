#include <stdlib.h>
#include <string.h>
#include "assemble_command.h"
#include "constant.h"
#include "error.h"
#include "opcode.h"
#include "string_process.h"
#include "symbol.h"

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


assemble_table set_command_table(
		char *symbol,
		char *mnemonic, 
		char *param, 
		int opcode, 
		int format, 
		int loc, 
		int line
) {
	assemble_table table = {.symbol = {0}, .mnemonic = {0}, .loc = 0};
	if(symbol)
		strcpy(table.symbol, symbol);
	if(mnemonic)
		strcpy(table.mnemonic, mnemonic);
	if(param)
		strcpy(table.param1, param);
	table.opcode = opcode;
	table.format = format;
	table.loc = loc;
	table.line = line * 5;
	return table;
}

/**** Pass 1 ****/
assemble_table line_to_command(char* str, int* error, int* curr_loc, int line){
	/**** Usable registers
	 * A:0, X:1, S:4, T:5
	 */
	char *tmp;
	char symbol[MAX_INSTRUCTION] = {0};
	int i;
	int symbol_flag = 0;
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
		return set_command_table(NULL, tmp, str+2, -2, 0, -1, line);
	}

	do{
		/**** Is Directive? ****/
		for(i = 0; i < DIRECTIVE_NUM; i++){
			if(!strcmp(tmp, directives[i])) {
				return table_directive(tmp, symbol, i, symbol_flag, line, error, curr_loc);
			}
		}

		/**** Is mnemonic? ****/	
		if(tmp[0] == '+')
			node_tmp = search_mnemonic(tmp+1);
		else
			node_tmp = search_mnemonic(tmp);


		/**** Mnemonic ****/
		if(node_tmp)
			return table_mnemonic(node_tmp, tmp, symbol, *curr_loc, line, error);
		
		else{	
			if(!symbol_flag){ 
				if(find_at_symbol(tmp)){
					error_in_assemble(8, line*5);
					(*error)++;
				}
				if(tmp[0] >= '0' && tmp[0] <='9'){
					error_in_assemble(10, line*5);
					(*error)++;
				}
				else{
					strcpy(symbol,tmp);
				}	
				symbol_flag = 1;
			}
			else{
				error_in_assemble(18, line*5);
				(*error)++;
				return set_command_table(symbol, NULL, tmp, -2, 0, *curr_loc, line);
			}
		}
	} while((tmp = strtok(NULL, " ")));
	return set_command_table(symbol, NULL, tmp, -2, 0, *curr_loc, line);
}

/**** Directives ****/
/* Setting 
 * mnemonic : directive 
 * loc : 
 * line : line*5
 * opcode : -1 (BYTE, WORD) others : -2
 * format : size
 */
assemble_table table_directive(
		char *str,
		char *symbol,
		enum enum_directive type,
		int exist_symbol,
		int line,
		int *error,
		int *curr_loc
) {
	char *tmp;
	char param[MAX_STR_LENGTH] = {0};
	char mnemonic[MAX_INSTRUCTION] = {0};
	
	strcpy(mnemonic,str);
	if(!exist_symbol){
		if(type >= 3){
			error_in_assemble(12, line*5);
			(*error)++;
		}
	}

	else{
		if(type >= 3){
		/* Setting
		 * symbol : already exists
		 * mnemonic : directive(START, BYTE, WORD, RESB, RESW)
		 * loc : curr_loc
		 * line : line*5
		 * opcode : BYTE, WORD - -1
		 * 					START, RESB, RESW - -2
		 * format : START - 0, others - size relative.
		 * param1 : START, RESB, RESW, WORD - number
		 *          BYTE - C' ', X' '
		 */
			switch(type){
				case START:
					return direc_START(symbol, mnemonic, exist_symbol, line, error, curr_loc);
				case BYTE:
					return direc_BYTE(str, symbol, mnemonic, line, error, curr_loc);
				case WORD:
					return direc_WORD(symbol, mnemonic, line, error, *curr_loc);
				case RESB:
					return direc_RESB(symbol, mnemonic, line, error, *curr_loc);
				case RESW:
					return direc_RESW(symbol, mnemonic, line, error, *curr_loc);
				default:
					break;
			}
		}
	}
	
	if(!(tmp = strtok(NULL, " "))){
		error_in_assemble(2, line*5);
		(*error)++;
	}
	strcpy(param,tmp);

	/**** Need just one parameter ****/
	if((tmp = strtok(NULL, " "))) {
		error_in_assemble(1, line*5);
		(*error)++;
	}
	if(!strcmp(mnemonic,"START"))
		return direc_START(symbol, mnemonic, exist_symbol, line, error, curr_loc);

	return set_command_table(symbol, mnemonic, param, -2, 0, -1, line);
}

/**** Mnemonic ****/
/* Setting 
 * mnemonic : existing mnemonic
 * loc : curr_loc
 * line : line*5
 * opcode : mnemonic opcode
 * format : mnemonic format
 * param1 : parameter
 * use_X : use
 */
assemble_table table_mnemonic(
		hash_node *mnemonic_node,
		char *str,
		char *symbol,
		int loc,
		int line,
		int *error
) {
	assemble_table table = {.symbol = {0}, .mnemonic = {0}, .loc = 0};
	char *tmp;
	char mnemonic[MAX_INSTRUCTION] = {0};
	char param[MAX_STR_LENGTH] = {0};
	int opcode, format;
	int comma_checker;	
	/**** Format 4 Check ****/
	/**** format setting ****/

	strcpy(mnemonic, str);
	if(str[0] == '+'){
		if(mnemonic_node->format2 == 4)
			format = 4;
		else{
			error_in_assemble(17, line*5);
			//				fprintf(stderr,"Error : line %d This mnemonic doesn't use format 4\n",line * 5);
			(*error)++;
		}
	}
	else
		format = mnemonic_node->format1;
	
	/**** opcode setting ****/
	opcode = mnemonic_node->opcode;

	/**** First parameter ****/
	tmp = strtok(NULL, " ");
	
	/**** No parameter ****/
	/**** Format 1 ****/
	if(!tmp)
		return set_command_table(symbol, mnemonic, param, opcode, format, loc, line);

	if(format == 1){
		error_in_assemble(14, line*5);
		(*error)++;
	}
	
	/**** parameter 1 setting ****/
	strcpy(param, tmp);

	/**** If format is 2, check if it is register ****/
	/**** Format 2 ****/
	/**** reg1 setting ****/
	if(format == 2){
		return mnem_f2(tmp, symbol, mnemonic, param, opcode, format, loc, line, error);
	}

	table = set_command_table(symbol, mnemonic, param, opcode, format, loc, line);
	
	/**** Have to Check
	 * Is it have comma?
	 *    - Then, it must have additional parameter
	 *    - If format 3 or 4, additional parameter must be register X
	 *    - If format 2, additional parameter must be registers
	 *    - If format 1, it must be Error
	 */
	if((comma_checker = comma_check(tmp))){
		if((tmp = strtok(NULL, " "))){
			/**** More than 3 parameters ****/
			if(comma_check(tmp)){
				error_in_assemble(1, line*5);
				(*error)++;
			}
			/**** use_X setting ****/
			else if((format == 4 || format == 3)){
				if(strlen(tmp) != 1){
					error_in_assemble(16, line*5);
					(*error)++;
				}
				else if(strlen(tmp) == 1 && tmp[0] == 'X')
					table.use_X = 1;
			}
		}
		else{
			error_in_assemble(2, line*5);
			(*error)++;
		}
	}
	else{
		/**** No comma, but have parameter ****/
		if((tmp = strtok(NULL, " "))){
			error_in_assemble(4, line*5);
			(*error)++;
		}
	}

	return table;
}


assemble_table direc_START(
		char *symbol,
		char *mnemonic,
		int exist_symbol,
		int line,
		int *error,
		int *curr_loc
) {
	/* format, param1 */
	int comma_checker;
	int loc;
	int format, opcode;
	char param[MAX_STR_LENGTH] = {0};
	char *tmp;
	
	/* Have to Check : 
	 * have comma? If so, print error.
	 * is hexadecimal? If it isn't, print error.
	 * hexa is in range? If it isn't print error.
	 */
	if(exist_symbol){
		tmp = strtok(NULL, " ");
		comma_checker = comma_check(tmp);
		if(comma_checker){
			error_in_assemble(3, line*5);
			(*error)++;
		}
	}

	loc = is_hexa(tmp,1);
	if(loc < 0){
		error_in_assemble(7, line*5);
		//fprintf(stderr, "Error : line %d Invalid hexadecimal number.\n", line*5);
		(*error)++;
	}
	*curr_loc = loc;
	format = 0;
	opcode = -2;
	strcpy(param, tmp);
	if((tmp = strtok(NULL, " "))){
		error_in_assemble(1, line*5);
		//fprintf(stderr, "Error : line %d Too much commands.\n", line*5);
		(*error)++;
	}
	return set_command_table(symbol,mnemonic,param,opcode,format,loc,line);
}

/**** BYTE ****/
/* Symbol must exists.
 * BYTE : X'  '  <- up to 60, only odd characters as hexa
 *     or C'  '  <- up to 30 characters as ASCII
 */

/* Have to check : Too much/less characters, 
 *	               Doesn't exist X or C,
 *        				 Doesn't exist ' '
 */
assemble_table direc_BYTE(
		char *str,
		char *symbol,
		char *mnemonic,
		int line,
		int *error,
		int *curr_loc
) {
	int format, opcode;
	char param[MAX_STR_LENGTH] = {0};
	
	strcpy(param, str+strlen(str)+1);
	format = direc_byte_check(param);
	
	if(format <= 0){
		error_in_assemble(13 ,line*5);
		//fprintf(stderr, "Error : line %d Invalid BYTE value.\n", line*5);
		(*error)++;
	}
	
	opcode = -1;

	return set_command_table(symbol, mnemonic, param, opcode, format, *curr_loc, line);
}

/**** WORD ****/
/* only decimal number */
assemble_table direc_WORD(
		char *symbol,
		char *mnemonic,
		int line,
		int *error,
		int curr_loc
) {
	int comma_checker;
	int format, opcode;
	char param[MAX_STR_LENGTH] = {0};
	char *tmp = strtok(NULL, " ");

	comma_checker = comma_check(tmp);
	if(comma_checker){
		error_in_assemble(3, line*5);
		//fprintf(stderr, "Error : line %d Additional parameter doesn't need.\n", line*5);
		(*error)++;
	}

	format = is_decimal(tmp);
	if(format < 0){
		error_in_assemble(6, line*5);
		//fprintf(stderr, "Error : line %d Invalid decimal number.\n", line*5);
		(*error)++;
	}
	opcode = -1;
	strcpy(param, tmp);
	
	if((tmp = strtok(NULL, " "))){
		error_in_assemble(1, line*5);
		//fprintf(stderr, "Error : line %d Too much commands.\n", line*5);
		(*error)++;
	}

	return set_command_table(symbol, mnemonic, param, opcode, format, curr_loc, line);
}

/**** RESB, RESW ****/
/* Symbol must exists.
 * parameter : hexadecimal numbers.
 */
assemble_table direc_RESB(
		char *symbol,
		char *mnemonic,
		int line,
		int *error,
		int curr_loc
) {
	int comma_checker;
	int opcode, format;
	char param[MAX_STR_LENGTH] = {0};
	char *tmp = strtok(NULL, " ");

	comma_checker = comma_check(tmp);
	if(comma_checker){
		error_in_assemble(3, line*5);
		//fprintf(stderr, "Error : line %d Additional parameter doesn't need.\n", line*5);
		(*error)++;
	}

	format = is_decimal(tmp);
	if(format < 0){
		error_in_assemble(6, line*5);
		//fprintf(stderr, "Error : line %d Invalid decimal number.\n", line*5);
		(*error)++;
	}
	opcode = -2;
	strcpy(param, tmp);
	if((tmp = strtok(NULL, " "))){
		error_in_assemble(1, line*5);
		//fprintf(stderr, "Error : line %d Too much commands.\n", line*5);
		(*error)++;
	}

	return set_command_table(symbol, mnemonic, param, opcode, format, curr_loc, line);
}

assemble_table direc_RESW(
		char *symbol,
		char *mnemonic,
		int line,
		int *error,
		int curr_loc
) {
	int comma_checker;
	int opcode, format;
	char param[MAX_STR_LENGTH] = {0};
	char *tmp = strtok(NULL, " ");

	comma_checker = comma_check(tmp);
	if(comma_checker){
		error_in_assemble(3, line*5);
		//fprintf(stderr, "Error : line %d Additional parameter doesn't need.\n", line*5);
		(*error)++;
	}

	format = is_decimal(tmp) * 3;
	if(format < 0){
		error_in_assemble(6, line*5);
		//fprintf(stderr, "Error : line %d Invalid decimal number.\n", line*5);
		(*error)++;
	}
	opcode = -2;
	strcpy(param, tmp);
	if((tmp = strtok(NULL, " "))){
		error_in_assemble(1, line*5);
		//fprintf(stderr, "Error : line %d Too much commands.\n", line*5);
		(*error)++;
	}
	return set_command_table(symbol, mnemonic, param, opcode, format, curr_loc, line);
}

assemble_table mnem_f2(
		char *str,
		char *symbol,
		char *mnemonic,
		char *param,
		int opcode,
		int format,
		int loc,
		int line,
		int *error
) {
	assemble_table table = {.symbol = {0}, .mnemonic = {0}, .loc = 0};
	char *tmp;
	int comma_checker = comma_check(str);

	table = set_command_table(symbol, mnemonic, param, opcode, format, loc, line);
	// set reg1
	if(strlen(str) > 2){
		error_in_assemble(15, line*5);
		(*error)++;
	}
	else{
		if(strlen(str) == 2){
			if(comma_checker && bit_reg(str[0]) < 0){
				error_in_assemble(15, line*5);
				(*error)++;
			}
			else	
				table.reg1 = str[0];
		}
		else
			table.reg1 = str[0];
	}

	// TODO : reg2 can be a number(0 ~ 15)
	if((tmp = strtok(NULL, " "))){
		/**** No comma, but have parameter ****/
		if(!comma_checker){
			error_in_assemble(4, line*5);
			(*error)++;
		}
		else{
			if(strlen(tmp) != 1){
				error_in_assemble(15, line*5);
				(*error)++;
			}
			else if(bit_reg(tmp[0]) < 0){
				error_in_assemble(15, line*5);
				(*error)++;
			}
			else
				table.reg2 = tmp[0];
		}
	}
	return table;
}
