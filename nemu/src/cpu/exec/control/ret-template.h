#include "cpu/exec/template-start.h"

#define instr ret

make_helper(instr){
    cpu.eip = MEM_R(cpu.esp)-1; 
    cpu.esp += DATA_BYTE;
    print_asm("ret");
    return 1;
}


make_helper(ret_i_w){
    uint16_t len=decode_i_w(eip+1); 
    cpu.eip = MEM_R(cpu.esp)-1-len;
    cpu.esp += 4+op_src->val;
    print_asm(str(instr) " 0x%x", op_src->val);
    return len+1;
}


#include "cpu/exec/template-end.h"