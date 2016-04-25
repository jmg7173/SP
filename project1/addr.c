#include <string.h>

#include "addr.h"
#include "debug.h"
#include "error.h"
#include "run.h"
#include "string_process.h"

static int progaddr = 0;

int get_Addr(){return progaddr;}
int command_progaddr(){
	char *tmp;
	int addr;

	tmp = strtok(NULL, " ");
	if(!tmp)
		return 2;
	else if((addr = is_hexa(tmp,1)) == -1)
		return 3;
	
	if(comma_check(tmp))
		return 9;

	if((tmp = strtok(NULL, " ")))
		return 6;

	set_Addr(addr);
	return 0;
}
void set_Addr(int addr){
	progaddr = addr;
	set_start_addr(progaddr);
	init_curr_bp();
}
