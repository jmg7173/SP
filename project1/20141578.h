#ifndef _20141578_H
#define _20141578_H

#define HELP_COMMAND_NUM 10
#define MAX_COMMAND_LENGTH 511

void quit_program();
void delete_trailing_whitespace(char*);
int command_with_whitespace(char*);
int print_error(const char*, int);

#endif
