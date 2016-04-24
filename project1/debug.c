#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "debug.h"
#include "structures.h"
#include "string_process.h"

static node *bp_head = NULL;
static node *curr_bp;

int command_bp(){
	char *tmp;
	int addr;
	int error = 0;

	if((tmp = strtok(NULL," "))){
		if(!(strcmp(tmp,"clear")))
			init_bp(1);
		else{
			if((addr = is_hexa(tmp, 1)) == -1)
				return 3;
			else if(comma_check(tmp))
				return 9;
			add_at_bp(addr, &error);
			if(error)
				return 15;
			printf("\t[ok] create breakpoint %04X\n",addr);
			init_curr_bp();
		}
	}

	else
		print_bp();
	return 0;
}

void print_bp(){
	node *tmp = bp_head;
	printf("\tbreakpoint\n");
	printf("\t----------\n");
	while(tmp != NULL){
		printf("\t%04X\n",tmp->addr);
		tmp = tmp->next;
	}
}

void add_at_bp(int addr, int *error){
	node *tmp;
	node *new_node;
	if(bp_head == NULL){
		new_node = (node*)malloc(sizeof(node));
		new_node->addr = addr;
		new_node->next = NULL;
		bp_head = new_node;
	}
	else{
		node *prev = bp_head;
		
		while(prev->next != NULL){
			if(prev->addr < addr){
				tmp = prev->next;
				if(tmp->addr > addr)
					break;
			}
			else if(prev->addr == addr){
				(*error)++;
				return;
			}
			else
				break;
			prev = prev->next;
		}

		new_node = (node*)malloc(sizeof(node));
		new_node->addr = addr;
		if(prev == bp_head){
			if(prev->addr == addr){
				(*error)++;
				return;
			}
			else if(prev->addr < addr){
				new_node->next = prev->next;
				prev->next = new_node;
			}
			else{
				new_node->next = prev;
				bp_head = new_node;
			}
		}
		else if(prev->next == NULL){
			prev->next = new_node;
			new_node->next = NULL;
		}
		else{
			new_node->next = prev->next;
			prev->next = new_node;
		}
	}
}

void init_bp(int option){
	node *tmp = bp_head;
	while(tmp != NULL){
		tmp = bp_head->next;
		free(bp_head);
		bp_head = tmp;
	}
	if(option == 1)
		printf("\t[ok] clear all breakpoints\n");
}

int get_next_bp(){
	int addr = -1;
	if(curr_bp != NULL){
		addr = curr_bp->addr;
		curr_bp = curr_bp->next;
	}
	return addr;
}

void init_curr_bp(){
	curr_bp = bp_head;
}
