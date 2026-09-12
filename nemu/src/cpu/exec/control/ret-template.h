#include "cpu/exec/template-start.h"

#define instr ret

make_helper(instr){
    cpu.eip = MEM_R(cpu.esp); 
    cpu.esp += 4;
    print_asm_template1();
    return 0;
}
 
#include "cpu/exec/template-end.h"