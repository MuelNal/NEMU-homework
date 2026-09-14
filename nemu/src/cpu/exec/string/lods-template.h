#include "cpu/exec/template-start.h"

#define instr lods

make_helper(concat(lods_, SUFFIX)) {
	DATA_TYPE val = MEM_R(cpu.esi);
    REG(R_EAX) = val;
    if (cpu.eflags.DF == 0) 
        cpu.esi += DATA_BYTE;
    else                    
        cpu.esi -= DATA_BYTE;
    print_asm("lodsb %%ds:(%%esi), %%al");
    return 1;
}

#include "cpu/exec/template-end.h"