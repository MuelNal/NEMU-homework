#include "cpu/exec/template-start.h"

#define instr call

static void do_execute() {
	cpu.esp-=DATA_BYTE;
    MEM_W(cpu.esp,cpu.eip+DATA_BYTE+1);
    cpu.eip+=op_src->val;
    print_asm(str(instr) " 0x%x", cpu.eip + 1 + DATA_BYTE);
}

make_instr_helper(si)
make_instr_helper(rm)


/*
int call_i_l((swaddr_t eip)) {
	return idex(eip, decode_i_l, do_execute); 
int idex(swaddr_t eip, int (*decode)(swaddr_t), void (*execute) (void)) {
	int len = decode(eip + 1);
	execute();
	return len + 1;	// "1" for opcode
}
*/

#include "cpu/exec/template-end.h"