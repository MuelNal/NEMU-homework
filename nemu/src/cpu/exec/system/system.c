#include "cpu/exec/helper.h"
#include "cpu/decode/modrm.h"

#include "x86-inc/mmu.h"

#include "system.h"

static const char *sreg_name[4] = { "es", "cs", "ss", "ds" };

/* Load the segment descriptor indexed by `selector' into the invisible part
 * of the segment register `sreg'. Only `base' and `limit' are simulated. */
void load_sreg(uint8_t sreg, uint16_t selector) {
	SegDesc desc;
	Sreg *s = seg_reg(sreg);
	uint32_t hi, limit;

	/* The base address stored in GDTR is a linear address. */
	lnaddr_t addr = cpu.gdtr.base + (selector & ~0x7u);

	desc.limit_15_0 = lnaddr_read(addr, 2);
	desc.base_15_0 = lnaddr_read(addr + 2, 2);
	hi = lnaddr_read(addr + 4, 4);

	desc.base_23_16 = hi >> 0;
	desc.type = hi >> 8;
	desc.segment_type = hi >> 12;
	desc.privilege_level = hi >> 13;
	desc.present = hi >> 15;
	desc.limit_19_16 = hi >> 16;
	desc.soft_use = hi >> 20;
	desc.operation_size = hi >> 21;
	desc.pad0 = hi >> 22;
	desc.granularity = hi >> 23;
	desc.base_31_24 = hi >> 24;

	Assert(desc.present == 1, "segment descriptor(selector = 0x%04x) is not present\n", selector);

	s->selector = selector;
	s->cache.base = (uint32_t)(desc.base_31_24 << 24)
		| (uint32_t)(desc.base_23_16 << 16) | desc.base_15_0;

	limit = (uint32_t)(desc.limit_19_16 << 16) | desc.limit_15_0;
	if(desc.granularity == SEG_4KB_GRANULARITY) {
		/* the limit is in 4096-byte units */
		limit = (limit << 12) | 0xfff;
	}
	s->cache.limit = limit;
}

/* lgdt m16&32: 0F 01 /2 */
make_helper(lgdt) {
	ModR_M m;
	Operand rm;

	m.val = instr_fetch(eip + 1, 1);
	assert(m.opcode == 2);

	rm.size = 4;
	int len = load_addr(eip + 1, &m, &rm);

	cpu.gdtr.limit = swaddr_read(rm.addr, 2, SREG_DS);
	cpu.gdtr.base = swaddr_read(rm.addr + 2, 4, SREG_DS);

	print_asm("lgdt %s", rm.str);
	return len + 1;
}

/* mov r32, cr0/cr3: 0F 20 /r */
make_helper(mov_cr2r) {
	ModR_M m;
	m.val = instr_fetch(eip + 1, 1);
	assert(m.mod == 3);

	/* the reg field of ModR_M selects the control register */
	switch(m.opcode) {
		case 0: reg_l(m.R_M) = cpu.cr0.val; break;
		case 3: reg_l(m.R_M) = cpu.cr3.val; break;
		default: assert(0);
	}

	print_asm("mov %%cr%d, %%%s", (int)m.opcode, regsl[m.R_M]);
	return 2;
}

/* mov cr0/cr3, r32: 0F 22 /r */
make_helper(mov_r2cr) {
	ModR_M m;
	m.val = instr_fetch(eip + 1, 1);
	assert(m.mod == 3);

	switch(m.opcode) {
		case 0: cpu.cr0.val = reg_l(m.R_M); break;
		case 3: cpu.cr3.val = reg_l(m.R_M); break;
		default: assert(0);
	}

	print_asm("mov %%%s, %%cr%d", regsl[m.R_M], (int)m.opcode);
	return 2;
}

/* mov r/m16, Sreg: 8E /r */
make_helper(mov_rm2sreg) {
	Operand rm, reg;

	rm.size = 2;
	int len = read_ModR_M(eip + 1, &rm, &reg);

	/* CS can only be loaded by a far jump, see i386 manual */
	assert(reg.reg != SREG_CS);

	load_sreg(reg.reg, (uint16_t)rm.val);

	print_asm("movw %s, %%%s", rm.str, sreg_name[reg.reg]);
	return len + 1;
}

/* jmp ptr16:32 (far jump, "ljmp"): EA */
make_helper(ljmp) {
	swaddr_t target = instr_fetch(eip + 1, 4);
	uint16_t selector = instr_fetch(eip + 5, 2);

	load_sreg(SREG_CS, selector);
	cpu.eip = target - 6;

	print_asm("ljmp $0x%x,$0x%x", selector, target);
	return 6;
}
