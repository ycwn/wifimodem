
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>

#include "core/types.h"
#include "core/debug.h"
#include "core/str.h"
#include "core/util.h"
#include "core/logger.h"
#include "core/mempool.h"


u64 mempool_blocks[MEMPOOL_BLOCK_NUM][MEMPOOL_BLOCK_SIZE / sizeof(u64)];
u16 mempool_free;

uint mempool_lock_count;
uint mempool_unlock_count;
uint mempool_histogram[MEMPOOL_BLOCK_NUM];



void mempool_create()
{

	mempool_lock_count   = 0;
	mempool_unlock_count = 0;

	for (int n=0; n < MEMPOOL_BLOCK_NUM; n++)
		mempool_histogram[n] = 0;

	mempool_free = ~0;

}



void *mempool_lock()
{

	int block = __builtin_ffsl(mempool_free) - 1;

	if (block < 0) {

		LOGE("mempool: Memory pool exausted\n"); //FIXME: Cant use logger here
		abort();

	}

	mempool_free &= ~(1 << block);
	mempool_histogram[block]++;
	mempool_lock_count++;

	return &mempool_blocks[block][0];

}



void mempool_unlock(void *ptr)
{

	if ((u8*)ptr < (const u8*)mempool_blocks || (const u8*)ptr >= (((const u8*)mempool_blocks) + sizeof(mempool_blocks)) ) {

		LOGE("mempool: Attempt to unlock() with invalid pointer: %p\n", ptr); //FIXME: Cant use logger here
		abort();

	}

	int  block = ((u8*)ptr - (u8*)&mempool_blocks[0][0]) / sizeof(mempool_blocks[0]);
	uint mask  = 1 << block;

	if ((mempool_free & mask) != 0) {

		LOGE("mempool: Double unlock() detected on block %d\n", block); //FIXME: Cant use logger here
		abort();

	}

	mempool_free |= mask;
	mempool_unlock_count++;

}



void mempool_statistics()
{
}


