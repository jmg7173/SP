#ifndef _FILE_H_
#define _FILE_H_

#include <stdio.h>

FILE* open_file(int *err, char* filename, int is_internal);
int command_type();

#endif
