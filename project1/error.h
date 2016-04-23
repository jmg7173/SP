#ifndef _ERROR_H
#define _ERROR_H

int print_error(char*, int);
void error_in_assemble(int status, int line);
void error_in_loader(int status, char* filename);

#endif
