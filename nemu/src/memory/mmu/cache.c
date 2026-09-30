#include "memory/mmu/cache.h"

#define CACHE_WAYS 8
#define CACHE_BLOCKS 64
#define CACHE_BLOCK_BITS 6
#define CACHE_SETS 128
#define CACHE_SET_BITS 7 
#define CACHE_TAG 19

uint32_t dram_read(hwaddr_t, size_t);
void dram_write(hwaddr_t, size_t, uint32_t);

typedef struct {
	struct{
		uint32_t valid :1;
		uint32_t tag :CACHE_TAG;
		uint8_t data[CACHE_BLOCKS];
	}ways[8];
}si_set;

si_set cache_sets[CACHE_SETS];

void init_cache()
{
	int i,j,k;
	for (i = 0; i < CACHE_SETS; i++) {
		for (j = 0; j < 8; j++) {
            cache_sets[i].ways[j].valid = 0;
            cache_sets[i].ways[j].tag   = 0;

            for (k = 0; k < CACHE_BLOCKS; k++) {
                cache_sets[i].ways[j].data[k] = 0;
            }
        }
    }
}

void cache_update(hwaddr_t hwaddr, size_t len, uint32_t data)
{
	uint32_t sw_offset = hwaddr & (CACHE_BLOCKS - 1);
    uint32_t sw_index  = (hwaddr >> CACHE_BLOCK_BITS) & (CACHE_SETS - 1);
    uint32_t sw_tag    = hwaddr >> (CACHE_BLOCK_BITS + CACHE_SET_BITS);
	int i,way=-1;
	for(i=0;i<CACHE_WAYS;i++){
		if(cache_sets[sw_index].ways[i].valid==1&&cache_sets[sw_index].ways[i].tag==sw_tag){
			way=i;
			break;
		}
	}
	if(way>-1){
		if(sw_offset+len>CACHE_BLOCKS){
			int rem=sw_offset+len-CACHE_BLOCKS;
			for(i=0;i<len-rem;i++){
				cache_sets[sw_index].ways[way].data[sw_offset+i]=data&0xff;
				data>>=8;
			}
			cache_update(hwaddr+len-rem,rem,data);
		}
		else{			
			for(i=0;i<len;i++){
				cache_sets[sw_index].ways[way].data[sw_offset+i]=data&0xff;
				data>>=8;
			}
			
		}
	}
}

void cache_write(hwaddr_t hwaddr, size_t len, uint32_t data)
{

	dram_write(hwaddr, len, data);   
    cache_update(hwaddr, len, data);
}

uint32_t cache_read(hwaddr_t hwaddr, size_t len)
{
	uint32_t sw_offset = hwaddr & (CACHE_BLOCKS - 1);
    uint32_t sw_index  = (hwaddr >> CACHE_BLOCK_BITS) & (CACHE_SETS - 1);
    uint32_t sw_tag    = hwaddr >> (CACHE_BLOCK_BITS + CACHE_SET_BITS);
	int i,way=-1;
	for(i=0;i<CACHE_WAYS;i++){
		if(cache_sets[sw_index].ways[i].valid==1&&cache_sets[sw_index].ways[i].tag==sw_tag){
			way=i;
			break;
		}
	}
	uint32_t data=0;
	if(way>-1){
		if(sw_offset+len>CACHE_BLOCKS){
			int rem=sw_offset+len-CACHE_BLOCKS;
			for(i=0;i<len-rem;i++){
				data |= (uint32_t)cache_sets[sw_index].ways[way].data[sw_offset+i] << (8 * i);
			}
			uint32_t rest = cache_read(hwaddr + len - rem, rem);
			return data | (rest << ((len - rem) * 8));
		}
		else{
			for(i=0;i<len;i++){
				data |= (uint32_t)cache_sets[sw_index].ways[way].data[sw_offset+i] << (8 * i);
			}
			return data;
		}
	}

	static int next_way = 0;
	way = next_way;
	next_way = (next_way + 1) % CACHE_WAYS;

	uint32_t block_addr = hwaddr & ~(CACHE_BLOCKS - 1);
		for(i = 0; i < CACHE_BLOCKS; i++){
		cache_sets[sw_index].ways[way].data[i] = dram_read(block_addr + i, 1) & 0xff;
	}
	cache_sets[sw_index].ways[way].valid=1;
	cache_sets[sw_index].ways[way].tag=sw_tag;
	
	if(sw_offset+len>CACHE_BLOCKS){
		int rem=sw_offset+len-CACHE_BLOCKS;
		for(i=0;i<len-rem;i++){
			data |= (uint32_t)cache_sets[sw_index].ways[way].data[sw_offset+i] << (8 * i);
		}
		uint32_t rest = cache_read(hwaddr + len - rem, rem);
		return data | (rest << ((len - rem) * 8));
	}
	else{
		for(i=0;i<len;i++){
			data |= (uint32_t)cache_sets[sw_index].ways[way].data[sw_offset+i] << (8 * i);
		}
		return data;
	}
}