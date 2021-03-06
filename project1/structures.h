#ifndef _STRUCTURES_H_
#define _STRUCTURES_H_

#include "constant.h"

/**** shell_command.c ****/
typedef struct _NODE{
	int idx;
	char command[MAX_STR_LENGTH + 1];
	struct _NODE* next;
}history_node;

/**** opcode.c ****/
typedef struct _HASH_NODE{
	int opcode;
	char instr[MAX_INSTRUCTION];
	int format1;
	int format2;
	struct _HASH_NODE *next;
}hash_node;

typedef struct{
	int opcode;
	char instr[MAX_INSTRUCTION];
	int format1;
	int format2;
}opcode_table;

/**** assemble.c ****/
enum enum_directive{
	END=0,
	BASE, 
	NOBASE,
	START,
	BYTE,
	WORD,
	RESB,
	RESW
};


enum addr_type{
	SIMPLE = 0,
	IMMEDIATE,
	INDIRECT
};

enum calc_type{
	STORE = 0,
	LOAD = 1
};

typedef struct{
	char symbol[MAX_INSTRUCTION];
	char mnemonic[MAX_INSTRUCTION];
	int loc;
	int line;
	int opcode;
	int format;
	char param1[MAX_STR_LENGTH];
	char reg1;
	char reg2;
	int use_X;
	int objectcode;
	unsigned char *obj_byte;
	int byte_len;
}assemble_table;

typedef struct TABLE{
	char symbol[MAX_INSTRUCTION];
	int line;
	int loc;
	struct TABLE *next;
}symbol_table;

typedef struct ESTAB{
	char csect[MAX_INSTRUCTION];
	char symbol[MAX_INSTRUCTION];
	int addr;
	int length;
	struct ESTAB *next;
}extsym_table;

typedef struct _BP_NODE{
	int addr;
	struct _BP_NODE *next;
}node;

#endif
