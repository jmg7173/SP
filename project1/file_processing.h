#ifndef _FILE_PROCESSING_H_
#define _FILE_PROCESSING_H_

#include <stdio.h>
#include "constant.h"

// TODO : 구조체 이름
// 주석은 어떤식으로 저장?
// 명령어?를 어떻게 저장? 동적할당 배열 vs linked list
// symbol table을 hash로 해야하나? 아니면 linked list?
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
}assemble_table;

typedef struct TABLE{
	char symbol[MAX_INSTRUCTION];
	int line;
	int loc;
	struct TABLE *next;
}symbol_table;

FILE* open_file(int*, char*);
int command_symbol();
int command_type();
int command_assemble();
assemble_table line_to_command(char*, int*, int*, int);
symbol_table* find_at_symbol(const char*);
void add_at_symbol(const char*, int, int);

#endif
