#ifndef _20141578_H
#define _20141578_H

#define HELP_COMMAND_NUM 10
#define MAX_COMMAND_LENGTH 511
#define MAX_MEMORY 1048576
#define DEFAULT_READ_MEMORY 159 

char memory[MAX_MEMORY] = {0};
int curr_addr = 0; 

void delete_trailing_whitespace(char*);
int command_with_whitespace(char*);
int is_hexa(const char*, int);
int comma_check(const char*);
int is_in_range(const char);
void print_memory(int, int);
void print_error(const char*, int);

#endif
