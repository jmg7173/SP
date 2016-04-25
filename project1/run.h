#ifndef _RUN_H_
#define _RUN_H_

int command_run();
void set_end_addr(int addr);
void set_start_addr(int addr);
void init_reg();
void print_register();
int immediate(int xbpe, int data);
int indirect(int xbpe, int data, int byte, int option, int save);
int simple(int xbpe, int data, int byte, int option, int save);
void execute_opcode(int opcode, int ni, int xbpe, int data);

void STA(int ni, int xbpe, int data);
void STL(int ni, int xbpe, int data);
void STCH(int ni, int xbpe, int data);
void STX(int ni, int xbpe, int data);
void LDA(int ni, int xbpe, int data);
void LDB(int ni, int xbpe, int data);
void LDT(int ni, int xbpe, int data);
void LDX(int ni, int xbpe, int data);
void LDCH(int ni, int xbpe, int data);
void JSUB(int ni, int xbpe, int data);
void JEQ(int ni, int xbpe, int data);
void J(int ni, int xbpe, int data);
void JLT(int ni, int xbpe, int data);
void RSUB();
void COMP(int ni, int xbpe, int data);
void COMPR(int data);
void TD();
void RD();
void WD();
void CLEAR(int data);
void TIXR(int data);

#endif
