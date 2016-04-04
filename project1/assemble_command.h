#ifndef _ASSEMBLE_COMMAND_H_
#define _ASSEMBLE_COMMAND_H_

#include "structures.h"

assemble_table line_to_command(char*, int*, int*, int);

assemble_table set_command_table(
		char *symbol,
		char *mnemonic, 
		char *param, 
		int opcode, 
		int format, 
		int loc, 
		int line
);

assemble_table table_directive(
		char *str,
		char *symbol,
		enum enum_directive type,
		int exist_symbol,
		int line,
		int *error,
		int *curr_loc
);

assemble_table table_mnemonic(
		hash_node *mnemonic_node,
		char *str,
		char *symbol,
		int loc,
		int line,
		int *error
);

assemble_table direc_START(
		char *symbol,
		char* mnemonic,
		int exist_symbol,
		int line,
		int *error,
		int *curr_loc
);

assemble_table direc_BYTE(
		char *str,
		char *symbol,
		char *mnemonic,
		int line,
		int *error,
		int *curr_loc
);

assemble_table direc_WORD(
		char *symbol,
		char *mnemonic,
		int line,
		int *error,
		int curr_loc
);

assemble_table direc_RESB(
		char *symbol,
		char *mnemonic,
		int line,
		int *error,
		int curr_loc
);

assemble_table direc_RESW(
		char *symbol,
		char *mnemonic,
		int line,
		int *error,
		int curr_loc
);

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
);
#endif
