#include "cpu/exec/template-start.h"

#define instr jo

static void do_execute() {
	if(cpu.eflags.OF==1){
        cpu.eip += op_src->val;
    }
    print_asm(str(instr) " 0x%x", cpu.eip + 1 + DATA_BYTE);
}

make_instr_helper(si)

#undef instr

#define instr jno

static void do_execute() {
	if(cpu.eflags.OF!=1){
        cpu.eip += op_src->val;
    }
    print_asm(str(instr) " 0x%x", cpu.eip + 1 + DATA_BYTE);
}

make_instr_helper(si)

#undef instr

#define instr jb

static void do_execute() {
	if(cpu.eflags.CF==1){
        cpu.eip += op_src->val;
    }
    
    print_asm(str(instr) " 0x%x", cpu.eip + 1 + DATA_BYTE);
}

make_instr_helper(si)

#undef instr

#define instr jnb

static void do_execute() {
	if(cpu.eflags.CF!=1){
        cpu.eip += op_src->val;
    }
    
    print_asm(str(instr) " 0x%x", cpu.eip + 1 + DATA_BYTE);
}

make_instr_helper(si)

#undef instr

#define instr je

static void do_execute() {
	if(cpu.eflags.ZF==1){
         cpu.eip += op_src->val;
    }
    
    print_asm(str(instr) " 0x%x", cpu.eip + 1 + DATA_BYTE);
}

make_instr_helper(si)

#undef instr

#define instr jne

static void do_execute() {
	if(cpu.eflags.ZF!=1){
        cpu.eip += op_src->val;
    }
    
    print_asm(str(instr) " 0x%x", cpu.eip + 1 + DATA_BYTE);
}

make_instr_helper(si)

#undef instr

#define instr ja

static void do_execute() {
	if(cpu.eflags.CF==0&&cpu.eflags.ZF==0){
         cpu.eip += op_src->val;
    }
    
    print_asm(str(instr) " 0x%x", cpu.eip + 1 + DATA_BYTE);
}

make_instr_helper(si)

#undef instr

#define instr jna

static void do_execute() {
	if(cpu.eflags.CF==0&&cpu.eflags.ZF==0);
    else{
        cpu.eip += op_src->val;
    }
    print_asm(str(instr) " 0x%x", cpu.eip + 1 + DATA_BYTE);
}

make_instr_helper(si)

#undef instr
#define instr js

static void do_execute() {
	if(cpu.eflags.SF==1){
        cpu.eip += op_src->val;
    }

    print_asm(str(instr) " 0x%x", cpu.eip + 1 + DATA_BYTE);
}

make_instr_helper(si)

#undef instr

#define instr jns

static void do_execute() {
	if(cpu.eflags.SF!=1){
        cpu.eip += op_src->val;
    }
    
    print_asm(str(instr) " 0x%x", cpu.eip + 1 + DATA_BYTE);
}

make_instr_helper(si)

#undef instr

#define instr jp

static void do_execute() {
	if(cpu.eflags.PF==1){
        cpu.eip += op_src->val;
    }
    
    print_asm(str(instr) " 0x%x", cpu.eip + 1 + DATA_BYTE);
}

make_instr_helper(si)

#undef instr

#define instr jnp

static void do_execute() {
	if(cpu.eflags.PF!=1){
        cpu.eip += op_src->val;
    }
    
    print_asm(str(instr) " 0x%x", cpu.eip + 1 + DATA_BYTE);
}

make_instr_helper(si)

#undef instr
#define instr jl

static void do_execute() {
	if(cpu.eflags.SF!=cpu.eflags.OF){
        cpu.eip += op_src->val;
    }
    
    print_asm(str(instr) " 0x%x", cpu.eip + 1 + DATA_BYTE);
}

make_instr_helper(si)

#undef instr

#define instr jnl

static void do_execute() {
	if(cpu.eflags.SF!=cpu.eflags.OF);
    else{
        cpu.eip += op_src->val;
    }
    print_asm(str(instr) " 0x%x", cpu.eip + 1 + DATA_BYTE);
}

make_instr_helper(si)

#undef instr

#define instr jg

static void do_execute() {
	if(cpu.eflags.SF==cpu.eflags.OF&&cpu.eflags.ZF==0){
       cpu.eip += op_src->val;
    }
    
    print_asm(str(instr) " 0x%x", cpu.eip + 1 + DATA_BYTE);
}

make_instr_helper(si)

#undef instr

#define instr jng

static void do_execute() {
	if(cpu.eflags.SF==cpu.eflags.OF&&cpu.eflags.ZF==0);
    else{
        cpu.eip += op_src->val;
    }
    print_asm(str(instr) " 0x%x", cpu.eip + 1 + DATA_BYTE);
}

make_instr_helper(si)

#undef instr

#if DATA_BYTE==1
#define instr jecxz

static void do_execute() {
	if(cpu.ecx==0){
        cpu.eip += op_src->val;
    }
    print_asm(str(instr) " 0x%x", cpu.eip + 1 + DATA_BYTE);
}

make_instr_helper(si)

#undef instr
#endif

#include "cpu/exec/template-end.h"
