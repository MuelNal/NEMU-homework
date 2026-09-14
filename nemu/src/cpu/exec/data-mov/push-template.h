#include "cpu/exec/template-start.h"

#define instr push

static void do_execute() {
	if(DATA_BYTE == 1) {
		/* push imm8: sign-extend to 32 bits, push 4 bytes */
		cpu.esp -= 4;
		swaddr_write(cpu.esp, 4, (int8_t)op_src->val);
	}
	else {
		cpu.esp -= DATA_BYTE;
		MEM_W(cpu.esp, op_src->val);
	}
    print_asm_template1();
}

make_instr_helper(i)

#if DATA_BYTE == 2 || DATA_BYTE == 4
make_instr_helper(r)
make_instr_helper(rm)
#endif
/*
int call_i_l((swaddr_t eip)) {
	return idex(eip, decode_i_l, do_execute); 
}
*/

#include "cpu/exec/template-end.h"
