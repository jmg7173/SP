#ifndef _DEBUG_H_
#define _DEBUG_H_

int command_bp();
void print_bp();
void add_at_bp(int addr, int *error);
void init_bp(int option);
int get_next_bp(int curr);
void init_curr_bp();

#endif
