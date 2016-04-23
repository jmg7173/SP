#ifndef _LOADER_H_
#define _LOADER_H_

int command_loader();
int obj_record_H(char *str, char *csect, int csaddr, int *length);
void obj_record_D(char *str, char *csect, int csaddr, int diff);

#endif
