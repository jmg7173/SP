#include <stdio.h>

#include "debug.h"
#include "memory.h"
#include "opcode.h"
#include "structures.h"
#include "run.h"

static int curr_addr = 0;
static int end_addr = -1;
struct reg{
	int A;
	int X;
	int L;
	int PC;
	int B;
	int S;
	int T;
	int SW;
};

static struct reg reg_set;
static int bp;

int command_run(){
	int data;
	int ni;
	int xbpe;
	int flag_e;

	opcode_table opcode;	

	reg_set.PC = curr_addr;
	bp = get_next_bp();

	if(bp < curr_addr) 
		bp = -1;
	
	while(curr_addr < end_addr){
		flag_e = 0;
		xbpe = 0;
		ni = 0;
		data = 0;
		if(bp != -1 && curr_addr > bp){
			print_register();
			printf("\tStop at checkpoint[%04X]\n",bp);
			return 0;
		}
		data = get_memory(curr_addr);
		opcode = search_as_opcode(data & 0xFC);
		
		/**** Byte directive ****/
		if(opcode.format1 == 0){
			reg_set.PC++;
			curr_addr++;
			continue;
		}
		else{
			/**** format 3 ****/
			if(opcode.format1 == 3){
				ni = data & 0x3;
				xbpe = get_memory(curr_addr + 1) >> 4;
				if(xbpe & 0x01) flag_e = 1;
				
				curr_addr++;
				data = get_memory(curr_addr) & 0x0F;

				data = data << 8;
				curr_addr++;
				data += get_memory(curr_addr);
				if(flag_e){
					data = data << 8;
					curr_addr++;
					data += get_memory(curr_addr);
					reg_set.PC += 4;
				}
				else{
					reg_set.PC += 3;
				}
				curr_addr++;
				execute_opcode(opcode.opcode, ni, xbpe, data);
			}

			/**** format 2 ****/
			else{
				reg_set.PC += 2;
				curr_addr += 2;
				data = get_memory(curr_addr);
			}
		}
		curr_addr = reg_set.PC;
		getchar();
	}
	
	print_register();
	printf("\tEnd Program\n");
	return 0;
}

void set_end_addr(int addr){
	end_addr = addr;
}

void set_start_addr(int addr){
	curr_addr = addr;
}

void init_reg(){
	reg_set.A = 0;
	reg_set.X = -1 & 0xFFFFFF;
	reg_set.L = -1 & 0xFFFFFF;
	reg_set.PC = -1 & 0xFFFFFF;
	reg_set.B = -1 & 0xFFFFFF;
	reg_set.S = 0;
	reg_set.T = 0;
	reg_set.SW = 0;
}

void print_register(){
	printf("\t\tA : %06X X : %06X\n",reg_set.A, reg_set.X);
	printf("\t\tL : %06X PC : %06X\n",reg_set.L, reg_set.PC);
	printf("\t\tB : %06X S : %06X\n",reg_set.B, reg_set.S);
	printf("\t\tT : %06X\n",reg_set.T);
}

int immediate(int xbpe, int data){
	int value;	
	if(!(xbpe & 0x01) && (data & 0x800)){
		data |= 0xFFFFF000;
	}
	if(xbpe == 0)
		value = data;
	else if(xbpe &0x01)
		value = data;
	else if(xbpe & 0x02) // PC Relative
		value = (data + reg_set.PC);
	else if(xbpe & 0x04) // Base Relative
		value = (data + reg_set.B);
	return value;
}

int indirect(int xbpe, int data, int byte, int option, int save){
	int value;
	int addr = simple(xbpe, data, byte, LOAD, 0);
	if(byte == 1){
		if(option == LOAD){
			value = get_memory(addr);
			return value;
		}
		else{
			set_memory(addr, save);
			return 0;
		}
	}
	
	if(option == LOAD){
		value  = get_memory(addr);
		value  = value << 8;
		value += get_memory(addr + 1);
		value  = value << 8;
		value += get_memory(addr + 2);
		value  = value << 8;
	}
	
	else{
		set_memory(addr, ((save >> 16) & 0xFF));
		set_memory(addr+1, ((save >> 8) & 0xFF));
		set_memory(addr+2, (save & 0xFF));
		return 0;
	}

	return value;
}

int simple(int xbpe, int data, int byte, int option, int save){
	int value;
	int addr = data;

	if(byte == 3){
		if(data & 0x800){
			data |= 0xFFFFF000;
		}
	}
	if(xbpe & 0x08)
		addr = data + reg_set.X;
	if(xbpe & 0x02)
		addr += reg_set.PC;
	else if(xbpe & 0x04)
		addr += reg_set.B;
	
	if(byte == 1){
		if(option == LOAD){
			value = get_memory(addr);
			return value;
		}
		else{
			set_memory(addr, save);
			return 0;
		}
	}
	
	if(option == LOAD){
		value  = get_memory(addr);
		value  = value << 8;
		value += get_memory(addr + 1);
		value  = value << 8;
		value += get_memory(addr + 2);
		value  = value << 8;
	}
	
	else{
		set_memory(addr, ((save >> 16) & 0xFF));
		set_memory(addr+1, ((save >> 8) & 0xFF));
		set_memory(addr+2, (save & 0xFF));
		return 0;
	}
	return value;
}

void execute_opcode(int opcode, int ni, int xbpe, int data){
	switch(opcode){
		case 0x00:
			LDA(ni, xbpe, data);
			break;

		case 0x68:
			LDB(ni, xbpe, data);
			break;

		case 0x74:
			LDT(ni, xbpe, data);
			break;

		case 0x50:
			LDCH(ni, xbpe, data);
			break;

		case 0x0C:
			STA(ni, xbpe, data);
			break;

		case 0x14:
			STL(ni, xbpe, data);
			break;

		case 0x10:
			STX(ni, xbpe, data);
			break;

		case 0x54:
			STCH(ni, xbpe, data);
			break;

		case 0x48:
			JSUB(ni, xbpe, data);
			break;

		case 0x30:
			JEQ(ni, xbpe, data);
			break;

		case 0x38:
			JLT(ni, xbpe, data);
			break;

		case 0x3C:
			J(ni, xbpe, data);
			break;

	case 0x28:
			COMP(ni, xbpe, data);
			break;

		case 0xE0:
			TD();
			break;

		case 0xD8:
			RD();
			break;

		case 0x4C:
			RSUB();
			break;

		case 0xDC:
			WD();
			break;

			/**** Format 2 ****/
		case 0xB4:
			CLEAR(data);
			break;

		case 0xA0:
			COMPR(data);
			break;

		case 0xB8:
			break;
	}
}

void STA(int ni, int xbpe, int data){
	if(ni == 2){
		indirect(xbpe, data, 3, STORE, reg_set.A);
	}
	else if(ni == 3){
		simple(xbpe, data, 3, STORE, reg_set.A);
	}
}

void STL(int ni, int xbpe, int data){
	if(ni == 2){
		indirect(xbpe, data, 3, STORE, reg_set.L);
	}
	else if(ni == 3){
		simple(xbpe, data, 3, STORE, reg_set.L);
	}
}

void STCH(int ni, int xbpe, int data){
	if(ni == 2){
		indirect(xbpe, data, 1, STORE, reg_set.A & 0xFF);
	}
	else if(ni == 3){
		simple(xbpe, data, 1, STORE, reg_set.A & 0xFF);
	}
}

void STX(int ni, int xbpe, int data){
	if(ni == 2){
		indirect(xbpe, data, 3, STORE, reg_set.X);
	}
	else if(ni == 3){
		simple(xbpe, data, 3, STORE, reg_set.X);
	}
}

void LDA(int ni, int xbpe, int data){
	if(ni == 1){
		reg_set.A = immediate(xbpe, data);
	}
	else if(ni == 2){
		reg_set.A = indirect(xbpe, data, 3, LOAD, 0);
	}
	else if(ni == 3){
		reg_set.A = simple(xbpe, data, 3, LOAD, 0);
	}
}

void LDB(int ni, int xbpe, int data){
	if(ni == 1){
		reg_set.B = immediate(xbpe, data);
	}
	else if(ni == 2){
		reg_set.B = indirect(xbpe, data, 3, LOAD, 0);
	}
	else if(ni == 3){
		reg_set.B = simple(xbpe, data, 3, LOAD, 0);
	}
}

void LDT(int ni, int xbpe, int data){
	if(ni == 1){
		reg_set.T = immediate(xbpe, data);
	}
	else if(ni == 2){
		reg_set.T = indirect(xbpe, data, 3, LOAD, 0);
	}
	else if(ni == 3){
		reg_set.T = simple(xbpe, data, 3, LOAD, 0);
	}
}

void LDCH(int ni, int xbpe, int data){
	if(ni == 1){
		reg_set.A = immediate(xbpe, data);
	}
	else if(ni == 2){
		reg_set.A = indirect(xbpe, data, 1, LOAD, 0);
	}
	else if(ni == 3){
		reg_set.A = simple(xbpe, data, 1, LOAD, 0);
	}
}

void JSUB(int ni, int xbpe, int data){
	int value;
	if(ni == 2)
		value = simple(xbpe, data, 3, LOAD, 0);
	if(ni == 3)
		value = immediate(xbpe, data);

	reg_set.L = reg_set.PC;
	reg_set.PC = value;
	
	init_curr_bp();
	while(bp < reg_set.PC) {
		bp = get_next_bp();
		printf("bp : %06X\n",bp);
		if(bp == -1) break;
	}
}

void JEQ(int ni, int xbpe, int data){
	int value;
	if(reg_set.SW == 0){
		if(ni == 2)
			value = simple(xbpe, data, 3, LOAD, 0);
		if(ni == 3)
			value = immediate(xbpe, data);

		reg_set.PC = value;
		init_curr_bp();
		while(bp < reg_set.PC) {
			bp = get_next_bp();
			if(bp == -1) break;
		}
	}
}

void J(int ni, int xbpe, int data){
	int value;	
	if(ni == 2)
		value = simple(xbpe, data, 3, LOAD, 0);
	if(ni == 3)
		value = immediate(xbpe, data);

	reg_set.PC = value;
	init_curr_bp();
	while(bp < reg_set.PC) {
		bp = get_next_bp();
		if(bp == -1) break;
	}
}

void JLT(int ni, int xbpe, int data){
	int value;
	if(reg_set.SW < 0){
		if(ni == 2)
			value = simple(xbpe, data, 3, LOAD, 0);
		if(ni == 3)
			value = immediate(xbpe, data);

		reg_set.PC = value;
		init_curr_bp();
		while(bp < reg_set.PC) {
			bp = get_next_bp();
			if(bp == -1) break;
		}
	}
}

void RSUB(){
	reg_set.PC = reg_set.L;
	init_curr_bp();
	while(bp < reg_set.PC) {
		bp = get_next_bp();
		if(bp == -1) break;
	}
}

void COMP(int ni, int xbpe, int data){
	int value;
	if(ni == 1){
		value = immediate(xbpe, data);
	}
	else if(ni == 2){
		value = indirect(xbpe, data, 3, LOAD, 0);
	}
	else if(ni == 3){
		value = simple(xbpe, data, 3, LOAD, 0);
	}

	if(reg_set.A > value)
		reg_set.SW = 1;
	else if(reg_set.A == value)
		reg_set.SW = 0;
	else
		reg_set.SW = -1;
}

void COMPR(int data){
	int reg_value1, reg_value2;
	switch(data >> 4){
		case 0:
			reg_value1 = reg_set.A;
			break;
		case 2:
			reg_value1 = reg_set.L;
			break;
		case 3:
			reg_value1 =	reg_set.B;
			break;
		case 4:
			reg_value1 =	reg_set.S;
			break;
		case 5:
			reg_value1 =	reg_set.T;
			break;
	}
	
	switch(data & 0x0F){
		case 0:
			reg_value2 = reg_set.A;
			break;
		case 2:
			reg_value2 = reg_set.L;
			break;
		case 3:
			reg_value2 =	reg_set.B;
			break;
		case 4:
			reg_value2 =	reg_set.S;
			break;
		case 5:
			reg_value2 =	reg_set.T;
			break;
	}

	if(reg_value1 > reg_value2)
		reg_set.SW = 1;
	else if(reg_value1 == reg_value2)
		reg_set.SW = 0;
	else reg_set.SW = -1;
}

void TD(){

}

void RD(){

}

void WD(){

}

void CLEAR(int data){
	switch(data >> 4){
		case 0:
			reg_set.A = 0;
			break;
		case 1:
			reg_set.X = 0;
			break;
		case 2:
			reg_set.L = 0;
			break;
		case 3:
			reg_set.B = 0;
			break;
		case 4:
			reg_set.S = 0;
			break;
		case 5:
			reg_set.T = 0;
			break;
	}
}

void TIXR(int data){
	int reg_value;
	reg_set.X++;
	switch(data >> 4){
		case 0:
			reg_value = reg_set.A;
			break;
		case 2:
			reg_value = reg_set.L;
			break;
		case 3:
			reg_value =	reg_set.B;
			break;
		case 4:
			reg_value =	reg_set.S;
			break;
		case 5:
			reg_value =	reg_set.T;
			break;
	}
	if(reg_set.X > reg_value)
		reg_set.SW = 1;
	else if(reg_set.X == reg_value)
		reg_set.SW = 0;
	else reg_set.SW = -1;
}
