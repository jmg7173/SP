#ifndef _LOADER_H_
#define _LOADER_H_

#include <stdio.h>

int command_loader();
int obj_record_H(char *str, char *csect, int csaddr, int *length, int *error);
void obj_record_D(char *str, char *csect, int diff, int *error);
void obj_record_R(char *str, int* modif, int* error);
void obj_record_T(char *str, int diff);
void obj_record_M(char *str, int *modif, int diff);
void memfree_loader(int tot, char** filename_arr, FILE **fp_arr);

#endif
