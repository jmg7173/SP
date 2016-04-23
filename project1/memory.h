#ifndef _MEMORY_H_
#define _MEMORY_H_

int command_dump();
int command_edit();
int command_fill();
void print_memory(int, int);
void reset_memory();
void set_memory(int addr, int value);
int get_memory(int addr);

#endif
