#include "cpu/exec/template-start.h"

#define instr jmp

static void do_execute() {
	cpu.eip += op_src->val;	//ops_decoded.src.val
	print_asm(str(instr) " %x", cpu.eip + 1 + DATA_BYTE);
}

make_instr_helper(si)		

/*
	make_helper(jmp_si_l) {
		return idex(eip, concat4(decode_, type, _, SUFFIX), do_execute);
	}

	int jmp_si_l(swaddr_t eip){
		return idex(eip, decode_si_l, do_execute);
	}

	int idex(swaddr_t eip, int (*decode)(swaddr_t), void (*execute) (void)) {
		int len = decode(eip + 1);
		execute();
		return len + 1;	// "1" for opcode
	}
*/

#if DATA_BYTE == 4
make_helper(jmp_rm_l) {
	int len = decode_rm_l(eip + 1);
	cpu.eip = op_src->val - (len + 1);
	print_asm(str(instr) " *%s", op_src->str);
	return len + 1;
}
#endif
#include "cpu/exec/template-end.h"
