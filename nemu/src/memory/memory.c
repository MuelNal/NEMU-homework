#include "nemu.h"

uint32_t cache_read(hwaddr_t hwaddr, size_t len);
void cache_write(hwaddr_t hwaddr, size_t len, uint32_t data);

/* Memory accessing interfaces */

uint32_t hwaddr_read(hwaddr_t addr, size_t len) {
	return cache_read(addr, len) & (~0u >> ((4 - len) << 3));
}

void hwaddr_write(hwaddr_t addr, size_t len, uint32_t data) {
	cache_write(addr, len, data);
}

uint32_t lnaddr_read(lnaddr_t addr, size_t len) {
	return hwaddr_read(addr, len);
}

void lnaddr_write(lnaddr_t addr, size_t len, uint32_t data) {
	hwaddr_write(addr, len, data);
}

/* Convert a virtual address into a linear address. Segmentation is only
 * enabled after the PE bit of CR0 is set. */
lnaddr_t seg_translate(swaddr_t addr, size_t len, uint8_t sreg) {
	Sreg *s;
	uint32_t end;

	if(cpu.cr0.protect_enable == 0) { return addr; }

	s = seg_reg(sreg);
	end = addr + len - 1;
	Assert(end <= s->cache.limit,
			"segment %d: address(0x%08x) exceeds the segment limit(0x%08x)",
			sreg, end, s->cache.limit);

	return s->cache.base + addr;
}

uint32_t swaddr_read(swaddr_t addr, size_t len, uint8_t sreg) {
#ifdef DEBUG
	assert(len == 1 || len == 2 || len == 4);
#endif
	return lnaddr_read(seg_translate(addr, len, sreg), len);
}

void swaddr_write(swaddr_t addr, size_t len, uint32_t data, uint8_t sreg) {
#ifdef DEBUG
	assert(len == 1 || len == 2 || len == 4);
#endif
	lnaddr_write(seg_translate(addr, len, sreg), len, data);
}

