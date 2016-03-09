#include "shell_command.h"
#include <dirent.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void print_curr_directory(){
	DIR *dir;
	struct dirent *files;
	struct stat info;
	mode_t state;

	dir = opendir(".");
	while( (files = readdir(dir)) != NULL){
		if(strcmp(files->d_name,".") == 0 ||
				strcmp(files->d_name,"..") == 0)
			continue;
		printf("%-s",files->d_name);
		stat(files->d_name, &info);
		state = info.st_mode;
		if(S_ISDIR(state))
			printf("/\t");
		else if(S_IXUSR & state)
			printf("*\t");
		else
			printf("\t");
	}
	printf("\n");
	closedir(dir);
}

void print_help_commands(){
	int i;
	for(i = 0; i<HELP_COMMAND_NUM; i++)
		printf("%s\n",help_commands[i]);
}

void add_to_history(const char* str){
	history_node* new_node;
	new_node = (history_node*)malloc(sizeof(history_node));
	strcpy(new_node->command, str);
	if(history_linked_list == NULL){
		new_node->idx = 1;
		history_linked_list = new_node;
		history_head = new_node; 
	}
	else{
		new_node->idx = history_linked_list->idx + 1;
		history_linked_list->next = new_node;
		history_linked_list = new_node;
		new_node->next = NULL;
	}
}

void print_history(){
	history_node* tmp;
	tmp = history_head;
	while(tmp != NULL){
		printf("%-4d %s\n",tmp->idx, tmp->command);
		tmp = tmp->next;
	}
}

void delete_history_linked_list(){
	history_node* tmp;
	tmp = history_head;
	while(tmp != NULL){
		tmp = history_head->next;
		free(history_head);
		history_head = tmp;
	}
}
