#include "common.h"
#include "memory.h"
#include <string.h>
#include "x86.h"

#define VMEM_ADDR 0xa0000
#define SCR_SIZE (320 * 200)

/* Use the function to get the start address of user page directory. */
PDE* get_updir();

/* The virtual address of the video memory lies in the page directory entry 0
 * (which covers [0x00000000, 0x003fffff)), and its page table index is
 * VMEM_ADDR >> 12. 320*200 bytes only occupy 16 pages, so the page table below
 * is written only partially, the other entries stay invalid.
 */
#define VMEM_PAGES   ((SCR_SIZE + PAGE_SIZE - 1) / PAGE_SIZE)
#define VMEM_PTE_IDX (VMEM_ADDR >> 12)

/* The page table which maps the video memory of the user program.
 * It must be page aligned, and it must be located in the physical memory below
 * 16MB, so mm_malloc() cannot be used to allocate it.
 */
static PTE vmem_ptable[NR_PTE] align_to_page;

void create_video_mapping() {
	PDE *updir = get_updir();
	int i;

	/* Make all the page table entries invalid at first. */
	memset(vmem_ptable, 0, sizeof(vmem_ptable));

	/* create an identical mapping from virtual memory area
	 * [0xa0000, 0xa0000 + SCR_SIZE) to physical memory area
	 * [0xa0000, 0xa0000 + SCR_SIZE) for user program. */
	for(i = 0; i < VMEM_PAGES; i ++) {
		vmem_ptable[VMEM_PTE_IDX + i].val = make_pte(VMEM_ADDR + i * PAGE_SIZE);
	}

	/* Let the corresponding page directory entry of the user process point to
	 * the page table above. The kernel is already running on the virtual
	 * address of `vmem_ptable', so it must be converted to a physical address
	 * before filling the page directory entry.
	 */
	updir[VMEM_ADDR / PT_SIZE].val = make_pde(va_to_pa(vmem_ptable));
}

void video_mapping_write_test() {
	int i;
	uint32_t *buf = (void *)VMEM_ADDR;
	for(i = 0; i < SCR_SIZE / 4; i ++) {
		buf[i] = i;
	}
}

void video_mapping_read_test() {
	int i;
	uint32_t *buf = (void *)VMEM_ADDR;
	for(i = 0; i < SCR_SIZE / 4; i ++) {
		assert(buf[i] == i);
	}
}

void video_mapping_clear() {
	memset((void *)VMEM_ADDR, 0, SCR_SIZE);
}

