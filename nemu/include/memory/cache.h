#ifndef __CACHE_H__
#define __CACHE_H__

#include "common.h"

#define cache_L1_size 64 * 1024
#define cache_L1_block_size 64
#define cache_L1_way_bit 3
#define cache_L1_group_bit 7
#define cache_L1_block_bit 6
#define cache_L1_group_size 1 << cache_L1_group_bit
#define cache_L1_way_size 1 << cache_L1_way_bit

#define Test

typedef struct
{
	uint8_t data[cache_L1_block_size];
	uint32_t tag;
	bool valid;
}cache_L1;

cache_L1 cache1[cache_L1_size/cache_L1_block_size];

#define cache_L2_size 4 * 1024 * 1024
#define cache_L2_block_size 64
#define cache_L2_way_bit 4
#define cache_L2_group_bit 12
#define cache_L2_block_bit 6
#define cache_L2_group_size (1 << cache_L2_group_bit)
#define cache_L2_way_size (1 << cache_L2_way_bit)

typedef struct
{
	uint8_t data[cache_L2_block_size];
	uint32_t tag;
	bool valid;
	bool dirty;
}cache_L2;

cache_L2 cache2[cache_L2_size/cache_L2_block_size];

#ifdef Test
int test_time;
#endif

void init_cache();
int read_cache1(hwaddr_t);
void write_cache1(hwaddr_t,size_t,uint32_t);
void write_cache2(hwaddr_t,size_t,uint32_t);
int read_cache2(hwaddr_t);
#endif
	
