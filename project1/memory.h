#ifndef _MEMORY_H_
#define _MEMORY_H_

#define MAX_MEMORY 1048576
#define DEFAULT_READ_MEMORY 159 

int command_dump();
int command_edit();
int command_fill();
int is_hexa(const char*, int);
int comma_check(const char*);
int is_in_range(const char);
void print_memory(int, int);
#endif
