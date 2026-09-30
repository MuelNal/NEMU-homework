#ifndef __CACHE_H__
#define __CACHE_H__

#include "nemu.h"

void init_cache();

void cache_write(hwaddr_t paddr, size_t len, uint32_t data);

uint32_t cache_read(hwaddr_t paddr, size_t len);

#endif