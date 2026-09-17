#include "cpu/exec/template-start.h"

#define instr call

static void do_execute() {
	cpu.esp-=DATA_BYTE;
    MEM_W(cpu.esp,cpu.eip+DATA_BYTE+1);
    cpu.eip+=op_src->val;
    print_asm(str(instr) " 0x%x", cpu.eip + 1 + DATA_BYTE);
}

make_instr_helper(si)

#if DATA_BYTE == 4
make_helper(call_rm_l) {
	int len = decode_rm_l(eip + 1);
	cpu.esp -= 4;
	MEM_W(cpu.esp, eip + len + 1);
	cpu.eip = op_src->val - (len + 1);
	print_asm(str(instr) " *%s", op_src->str);
	return len + 1;
}
#endif

#if DATA_BYTE == 2
make_helper(call_rm_w) {
	int len = decode_rm_w(eip + 1);
	cpu.esp -= 2;
	MEM_W(cpu.esp, eip + len + 1);
	cpu.eip = op_src->val - (len + 1);
	print_asm(str(instr) " *%s", op_src->str);
	return len + 1;
}
#endif


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