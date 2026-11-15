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

/* Convert a linear address into a physical address by a page walk.
 * Paging is enabled only after both the PE bit and the PG bit of CR0
 * are set, so it is checked in restart() and in every access. */

/* Do the page walk without aborting NEMU, so that the monitor command `page'
 * can report the reason of a failed translation to the user:
 * the physical address is stored in *pa, and one of the PG_* status codes is
 * returned. When paging is disabled the linear address is used as it is. */
int page_walk(lnaddr_t addr, hwaddr_t *pa) {
	PDE pde;
	PTE pte;
	uint32_t pdir_idx, ptable_idx;

	/* paging is enabled only when both PE and PG are set, see i386 manual */
	if(cpu.cr0.protect_enable == 0 || cpu.cr0.paging == 0) {
		*pa = addr;
		return PG_NO_PAGING;
	}

	pdir_idx = (addr >> 22) & 0x3ff;
	ptable_idx = (addr >> 12) & 0x3ff;

	/* The base addresses in PDE/PTE/CR3 are physical addresses. */
	pde.val = hwaddr_read((cpu.cr3.page_directory_base << 12) + pdir_idx * 4, 4);
	if(pde.present == 0) { return PG_PDE_ABSENT; }

	pte.val = hwaddr_read((pde.page_frame << 12) + ptable_idx * 4, 4);
	if(pte.present == 0) { return PG_PTE_ABSENT; }

	*pa = (pte.page_frame << 12) | (addr & 0xfff);
	return PG_OK;
}

hwaddr_t page_translate(lnaddr_t addr) {
	hwaddr_t pa = addr;
	int status = page_walk(addr, &pa);

	/* no paging: the linear address is the physical address */
	if(status == PG_NO_PAGING) { return pa; }

	Assert(status == PG_OK, "%s(linear address = 0x%08x)",
			status == PG_PDE_ABSENT ? "page directory entry is not present"
			: status == PG_PTE_ABSENT ? "page table entry is not present"
			: "page-level address translation failed", addr);
	return pa;
}

uint32_t lnaddr_read(lnaddr_t addr, size_t len) {
	if(cpu.cr0.paging) {
		/* 按照 KISS 法则, 暂不实现数据跨越虚拟页边界的情况 */
		Assert((addr & 0xfff) + len <= PAGE_SIZE,
				"read(addr = 0x%08x, len = %d) crosses the page boundary",
				addr, (int)len);
	}
	return hwaddr_read(page_translate(addr), len);
}

void lnaddr_write(lnaddr_t addr, size_t len, uint32_t data) {
	if(cpu.cr0.paging) {
		Assert((addr & 0xfff) + len <= PAGE_SIZE,
				"write(addr = 0x%08x, len = %d) crosses the page boundary",
				addr, (int)len);
	}
	hwaddr_write(page_translate(addr), len, data);
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

