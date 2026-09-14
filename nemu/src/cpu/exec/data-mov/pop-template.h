#include "cpu/exec/template-start.h"

#define instr pop

static void do_execute() {
	DATA_TYPE data=MEM_R(cpu.esp);
	OPERAND_W(op_src,data);
    cpu.esp+=DATA_BYTE;
    print_asm_template1();
}

make_instr_helper(r)
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