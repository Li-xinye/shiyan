#include "common.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "burst.h"
#include "memory/cache.h"

void ddr3_read_replace(hwaddr_t addr,void *data);
void dram_write(hwaddr_t addr,size_t len,uint32_t data);
void ddr3_write_replace(hwaddr_t addr,void *data,uint8_t *mask);

void init_cache()
{
	int i;
	for(i=0;i < cache_L1_size / cache_L1_block_size;i++)
		cache1[i].valid = 0;
	for(i=0;i < cache_L2_size / cache_L2_block_size;i++)
	{
		cache2[i].valid =0;
		cache2[i].dirty =0;
	}
	test_time = 0;
}

int read_cache1(hwaddr_t addr)
{
	uint32_t group_idx = (addr >> cache_L1_block_bit) & ((cache_L1_group_size) -1);
	uint32_t tag = (addr >> (cache_L1_group_bit + cache_L1_block_bit));
	int i,group = group_idx * cache_L1_way_size;
	for(i = group + 0;i < group + (cache_L1_way_size);i++)
	{
		if(cache1[i].valid == 1 && cache1[i].tag == tag)
		{
			#ifdef Test
				test_time += 2;
			#endif
				return i;
		}
	}
	int pl= read_cache2(addr);
	srand(time(0));
	i=group + rand()%(cache_L1_way_size);
	memcpy(cache1[i].data,cache2[pl].data,cache_L1_block_size);

	cache1[i].valid = 1;
	cache1[i].tag = tag;
	return 1;
	
}

int read_cache2(hwaddr_t addr)
{
	uint32_t group_idx = (addr >> cache_L2_block_bit) & ((cache_L2_group_size) -1);
        uint32_t tag = (addr >> (cache_L2_group_bit + cache_L2_block_bit));
	uint32_t block_start = (addr >> cache_L2_block_bit) << cache_L2_block_bit;

	int i,group = group_idx *cache_L2_way_size;
	for(i = group + 0;i<group+cache_L2_way_size;i++)
	{
		if(cache2[i].valid ==1 && cache2[i].tag == tag)
		{
			#ifdef Test
				test_time +=10;
			#endif
				return i;
		}
	}
#ifdef Test
	test_time += 200;
#endif
	srand(time(0));
	i = group + rand()%cache_L2_way_size;
	if(cache2[i].valid ==1 && cache2[i].dirty == 1)
	{
		uint8_t ret[BURST_LEN << 1];
		uint32_t block_st = (cache2[i].tag << (cache_L2_group_bit + cache_L2_block_bit)) | (group_idx << cache_L2_block_bit);
		int w;
		memset(ret,1,sizeof ret);
		for(w=0;w <cache_L2_block_size/BURST_LEN;w++)
			ddr3_write_replace(block_st +BURST_LEN * w,cache2[i].data + BURST_LEN *w,ret);
	}
	int j;
	for(j=0;j<cache_L2_block_size/BURST_LEN;j++)
		ddr3_read_replace(block_start + BURST_LEN * j,cache2[i].data + BURST_LEN*j);
	cache2[i].dirty =0;
	cache2[i].valid =1;
	cache2[i].tag =tag;
	return i;
}

void write_cache1( hwaddr_t addr,size_t len,uint32_t data)
{
	uint32_t group_idx = (addr >> cache_L1_block_bit) & ((cache_L1_group_size) -1);
        uint32_t tag = (addr >> (cache_L1_group_bit + cache_L1_block_bit));
	uint32_t offset = addr & (cache_L1_block_size -1);

	int i,group = group_idx *cache_L1_way_size;
	for(i=group + 0;i<group+(cache_L1_way_size);i++)
	{
		if(cache1[i].valid == 1 && cache1[i].tag ==tag)
		{
			if(offset + len > cache_L1_block_size)
			{
				dram_write(addr,cache_L1_block_size-offset,data);
				memcpy(cache1[i].data + offset,&data,cache_L1_block_size-offset);
				write_cache2(addr,cache_L1_block_size-offset,data);
				write_cache1(addr + cache_L1_block_size-offset,len-(cache_L1_block_size-offset),data>>(8*(cache_L1_block_size-offset)));
			}
			else
			{
				dram_write(addr,len,data);
				memcpy(cache1[i].data + offset,&data,len);
				write_cache2(addr,len,data);
			}
			return;
		}
	}
	write_cache2(addr,len,data);
}

void write_cache2(hwaddr_t addr,size_t len,uint32_t data)
{
	uint32_t group_idx = (addr >> cache_L2_block_bit) & (cache_L2_group_size -1);
        uint32_t tag = (addr >> (cache_L2_group_bit + cache_L2_block_bit));
        uint32_t offset = addr & (cache_L2_block_size -1);

        int i,group = group_idx *cache_L2_way_size;
	for(i=group+0;i<group + cache_L2_way_size;i++)
	{
		if(cache2[i].valid == 1 && cache2[i].tag==tag)
		{
			cache2[i].dirty = 1;
			if(offset + len > cache_L2_block_size)
			{
				memcpy(cache2[i].data+offset,&data,cache_L2_block_size - offset);
				write_cache2(addr + cache_L2_block_size - offset,len-(cache_L2_block_size-offset),data>>(8*(cache_L2_block_size-offset)));
			}
			else
			{
				memcpy(cache2[i].data+offset,&data,len);
			}
			return;
		}
	}
	i=read_cache2(addr);
	cache2[i].dirty = 1;
	memcpy(cache2[i].data+offset,&data,len);
}

