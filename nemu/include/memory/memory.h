#ifndef __MEMORY_H__
#define __MEMORY_H__

#include "common.h"

#define HW_MEM_SIZE (128 * 1024 * 1024)

extern uint8_t *hw_mem;

/* convert the hardware address in the test program to virtual address in NEMU */
#define hwa_to_va(p) ((void *)(hw_mem + (unsigned)p))
/* convert the virtual address in NEMU to hardware address in the test program */
#define va_to_hwa(p) ((hwaddr_t)((void *)p - (void *)hw_mem))

#define hw_rw(addr, type) *(type *)({\
	Assert(addr < HW_MEM_SIZE, "physical address(0x%08x) is out of bound", addr); \
	hwa_to_va(addr); \
})

uint32_t swaddr_read(swaddr_t, size_t, uint8_t);
uint32_t lnaddr_read(lnaddr_t, size_t);
uint32_t hwaddr_read(hwaddr_t, size_t);
void swaddr_write(swaddr_t, size_t, uint32_t, uint8_t);
void lnaddr_write(lnaddr_t, size_t, uint32_t);
void hwaddr_write(hwaddr_t, size_t, uint32_t);

/* segment-level address translation */
lnaddr_t seg_translate(swaddr_t addr, size_t len, uint8_t sreg);

/* page-level address translation */
hwaddr_t page_translate(lnaddr_t addr);

/* The status of a page walk, see page_walk(). */
enum { PG_OK = 0, PG_NO_PAGING, PG_PDE_ABSENT, PG_PTE_ABSENT };

/* Do a page walk for the monitor command `page'.
 * Unlike page_translate(), this function never aborts NEMU, so that the
 * reason of a failed translation can be reported to the user. */
int page_walk(lnaddr_t addr, hwaddr_t *pa);

#endif
