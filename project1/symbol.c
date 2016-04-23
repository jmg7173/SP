#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol.h"

static symbol_table *symbols = NULL;
static symbol_table *tmp_table = NULL;
static symbol_table *symbol_head;

static extsym_table *estab = NULL;

/**** Description : execute command symbol ****/
void command_symbol(){
	symbol_table *tmp = symbols;
	int cnt = 0;
	while(tmp != NULL){
		printf("\t%s\t%04X\n",tmp->symbol, tmp->loc);
		tmp = tmp->next;
		cnt++;
	}
	if(cnt == 0){
		printf("No symbols.\n");
	}
}

/**** Find symbol at tmp_table(tmp symbol table) ****/
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

/**** Add symbol at tmp symbol table ****/
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

/**** Add symbol at symbol table as descending order ****/
void add_at_symbol_table(const char* str, int curr_loc, int line){
	symbol_table *new_node;
	symbol_table *tmp = symbol_head;
	if(symbols == NULL){
		new_node = (symbol_table*)malloc(sizeof(symbol_table));
		new_node->line = line;
		new_node->loc = curr_loc;
		strcpy(new_node->symbol, str);
		new_node->next = NULL;
		symbols = new_node;
		symbol_head = symbols;
	}
	else{
		new_node = (symbol_table*)malloc(sizeof(symbol_table));
		new_node->line = line;
		new_node->loc = curr_loc;
		strcpy(new_node->symbol, str);
		while(tmp->next != NULL){
			if(strcmp(tmp->symbol, str) > 0){
				if(strcmp(tmp->next->symbol, str) < 0){
					new_node->next = tmp->next;
					tmp->next = new_node;
					break;
				}
				else
					tmp = tmp->next;
			}
			else{
				if(tmp == symbol_head){
					new_node->next = tmp;
					symbol_head = symbols = new_node;
					break;
				}
			}
		}
		if(tmp == symbol_head){
			if(strcmp(tmp->symbol, str) > 0){
				new_node = tmp->next;
				tmp->next = new_node;
			}
			else{
				new_node->next = tmp;
				symbol_head = symbols = new_node;
			}
		}
		else if(tmp->next == NULL){
			new_node->next = tmp->next;
			tmp->next = new_node;
		}
	}
}

/**** Free memory of symbol table ****/
void delete_at_symbol_table(){
	symbol_table *tmp;
	tmp = symbols;
	while(tmp != NULL){
		tmp = symbols->next;
		free(symbols);
		symbols = tmp;
	}
}

/**** Free memory of tmp symbol table ****/
void delete_at_tmp_symbol(int option){
	symbol_table *tmp;
	tmp = tmp_table;
	while(tmp != NULL){
		if(option == 1)
			add_at_symbol_table(tmp->symbol, tmp->loc, tmp->line);
		tmp = tmp_table->next;
		free(tmp_table);
		tmp_table = tmp;
	}
}

void add_at_estab(
		const char *csect, 
		const char *symbol,
		int addr,
		int length
){
	extsym_table *new_node;
	extsym_table *tmp = estab;
	if(tmp == NULL){
		new_node = (extsym_table*)malloc(sizeof(extsym_table));
		strcpy(new_node->csect, csect);
		strcpy(new_node->symbol, symbol);
		new_node->addr = addr;
		new_node->length = length;
		new_node->next = NULL;
		estab = new_node;
	}
	else{
		new_node = (extsym_table*)malloc(sizeof(extsym_table));
		while(tmp->next != NULL){
			tmp = tmp->next;
		}
		strcpy(new_node->csect, csect);
		strcpy(new_node->symbol, symbol);
		new_node->addr = addr;
		new_node->length = length;
		new_node->next = NULL;
		tmp->next = new_node;
	}
}

void print_estab(){
	extsym_table *tmp = estab;
	int total_length = 0;

	printf("control\t\tsymbol\t\taddress\t\tlength\n");
	printf("section\t\tname\n");
	printf("---------------------------------------------------------\n");
	while(tmp != NULL){
		if(!strcmp(tmp->csect,tmp->symbol)){
			printf("%s\t\t\t\t%04X\t\t%04X\n",
					tmp->csect,
					tmp->addr,
					tmp->length
			);
			total_length += tmp->length;
		}
		else{
			printf("\t\t%s\t\t%04X\n",
					tmp->symbol,
					tmp->addr
			);
		}
		tmp = tmp->next;
	}
	printf("---------------------------------------------------------\n");
	printf("\t\t\t\ttotal length\t%04X\n",total_length);
}

void init_estab(){
	extsym_table *tmp;
	tmp = estab;
	while(tmp != NULL){
		tmp = estab->next;
		free(estab);
		estab = tmp;
	}
	estab = NULL;
}
