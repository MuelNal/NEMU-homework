#include "memory/mmu/cache.h"

#define CACHE_WAYS 16
#define CACHE_BLOCKS 64
#define CACHE_BLOCK_BITS 6
#define CACHE_SETS 4096
#define CACHE_SET_BITS 12 
#define CACHE_TAG 14

static int next_way = 0;

uint32_t dram_read(hwaddr_t, size_t);
void dram_write(hwaddr_t, size_t, uint32_t);

typedef struct {
	struct{
		uint32_t valid :1;
        uint32_t dirty :1;
		uint32_t tag :CACHE_TAG;
		uint8_t data[CACHE_BLOCKS];
	}ways[CACHE_WAYS];
}si_L2_set;

si_L2_set cache_L2_sets[CACHE_SETS];

void init_cache_L2()
{
	int i,j,k;
	for (i = 0; i < CACHE_SETS; i++) {
		for (j = 0; j < CACHE_WAYS; j++) {
            cache_L2_sets[i].ways[j].valid = 0;
            cache_L2_sets[i].ways[j].tag   = 0;
			cache_L2_sets[i].ways[j].dirty   = 1;

            for (k = 0; k < CACHE_BLOCKS; k++) {
                cache_L2_sets[i].ways[j].data[k] = 0;
            }
        }
    }
}

void cache_write_L2(hwaddr_t hwaddr, size_t len, uint32_t data)
{  
   	uint32_t sw_offset = hwaddr & (CACHE_BLOCKS - 1);
    uint32_t sw_index  = (hwaddr >> CACHE_BLOCK_BITS) & (CACHE_SETS - 1);
    uint32_t sw_tag    = hwaddr >> (CACHE_BLOCK_BITS + CACHE_SET_BITS);
	int i,way=-1;
	for(i=0;i<CACHE_WAYS;i++){
		if(cache_L2_sets[sw_index].ways[i].valid==1&&cache_L2_sets[sw_index].ways[i].tag==sw_tag){
			way=i;
			break;
		}
	}
	if(way>-1){
		cache_L2_sets[sw_index].ways[way].dirty=1;
		
		if(sw_offset+len>CACHE_BLOCKS){
			int rem=sw_offset+len-CACHE_BLOCKS;
			for(i=0;i<len-rem;i++){
				cache_L2_sets[sw_index].ways[way].data[sw_offset+i]=data&0xff;
				data>>=8;
			}
			cache_write_L2(hwaddr+len-rem,rem,data);
		}
		else{			
			for(i=0;i<len;i++){
				cache_L2_sets[sw_index].ways[way].data[sw_offset+i]=data&0xff;
				data>>=8;
			}
			
		}
	}
	else{
		way = next_way;
		next_way = (next_way + 1) % CACHE_WAYS;

		if(cache_L2_sets[sw_index].ways[way].valid&&cache_L2_sets[sw_index].ways[way].dirty){
			uint32_t old_addr = ((uint32_t)cache_L2_sets[sw_index].ways[way].tag << (CACHE_BLOCK_BITS + CACHE_SET_BITS)) | ((uint32_t)sw_index << CACHE_BLOCK_BITS);
			for(i=0;i<CACHE_BLOCKS;i++){
				dram_write(old_addr+i, 1, cache_L2_sets[sw_index].ways[way].data[i]);
			}
		}

		uint32_t block_addr = hwaddr & ~(CACHE_BLOCKS - 1);
		for(i = 0; i < CACHE_BLOCKS; i++){
			cache_L2_sets[sw_index].ways[way].data[i] = dram_read(block_addr + i, 1) & 0xff;
		}
		cache_L2_sets[sw_index].ways[way].valid=1;
		cache_L2_sets[sw_index].ways[way].tag=sw_tag;
		
		cache_L2_sets[sw_index].ways[way].dirty=1;
		
		if(sw_offset+len>CACHE_BLOCKS){
			int rem=sw_offset+len-CACHE_BLOCKS;
			for(i=0;i<len-rem;i++){
				cache_L2_sets[sw_index].ways[way].data[sw_offset+i]=data&0xff;
				data>>=8;
			}
			cache_write_L2(hwaddr+len-rem,rem,data);
		}
		else{			
			for(i=0;i<len;i++){
				cache_L2_sets[sw_index].ways[way].data[sw_offset+i]=data&0xff;
				data>>=8;
			}
			
		}
	}
}

uint32_t cache_read_L2(hwaddr_t hwaddr, size_t len)
{
	uint32_t sw_offset = hwaddr & (CACHE_BLOCKS - 1);
    uint32_t sw_index  = (hwaddr >> CACHE_BLOCK_BITS) & (CACHE_SETS - 1);
    uint32_t sw_tag    = hwaddr >> (CACHE_BLOCK_BITS + CACHE_SET_BITS);
	int i,way=-1;
	for(i=0;i<CACHE_WAYS;i++){
		if(cache_L2_sets[sw_index].ways[i].valid==1&&cache_L2_sets[sw_index].ways[i].tag==sw_tag){
			way=i;
			break;
		}
	}
	uint32_t data=0;
	if(way>-1){
		if(sw_offset+len>CACHE_BLOCKS){
			int rem=sw_offset+len-CACHE_BLOCKS;
			for(i=0;i<len-rem;i++){
				data |= (uint32_t)cache_L2_sets[sw_index].ways[way].data[sw_offset+i] << (8 * i);
			}
			uint32_t rest = cache_read_L2(hwaddr + len - rem, rem);
			return data | (rest << ((len - rem) * 8));
		}
		else{
			for(i=0;i<len;i++){
				data |= (uint32_t)cache_L2_sets[sw_index].ways[way].data[sw_offset+i] << (8 * i);
			}
			return data;
		}
	}

	way = next_way;
	next_way = (next_way + 1) % CACHE_WAYS;

	if(cache_L2_sets[sw_index].ways[way].valid&&cache_L2_sets[sw_index].ways[way].dirty){
		uint32_t old_addr = ((uint32_t)cache_L2_sets[sw_index].ways[way].tag << (CACHE_BLOCK_BITS + CACHE_SET_BITS)) | ((uint32_t)sw_index << CACHE_BLOCK_BITS);
		for(i=0;i<CACHE_BLOCKS;i++){
			dram_write(old_addr+i, 1, cache_L2_sets[sw_index].ways[way].data[i]);
		}
	}

	uint32_t block_addr = hwaddr & ~(CACHE_BLOCKS - 1);
		for(i = 0; i < CACHE_BLOCKS; i++){
		cache_L2_sets[sw_index].ways[way].data[i] = dram_read(block_addr + i, 1) & 0xff;
	}
	cache_L2_sets[sw_index].ways[way].valid=1;
	cache_L2_sets[sw_index].ways[way].dirty=0;
	cache_L2_sets[sw_index].ways[way].tag=sw_tag;
	
	if(sw_offset+len>CACHE_BLOCKS){
		int rem=sw_offset+len-CACHE_BLOCKS;
		for(i=0;i<len-rem;i++){
			data |= (uint32_t)cache_L2_sets[sw_index].ways[way].data[sw_offset+i] << (8 * i);
		}
		uint32_t rest = cache_read_L2(hwaddr + len - rem, rem);
		return data | (rest << ((len - rem) * 8));
	}
	else{
		for(i=0;i<len;i++){
			data |= (uint32_t)cache_L2_sets[sw_index].ways[way].data[sw_offset+i] << (8 * i);
		}
		return data;
	}
}