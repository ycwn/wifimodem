
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>

#include "core/types.h"
#include "core/debug.h"
#include "core/str.h"
#include "core/logger.h"
#include "core/arena.h"


enum {
	ARENA_ALIGNMENT = 16
};


void *g_arena       = NULL;
uint  g_arena_size  = 0;
uint  g_arena_point = 0;



void arena_create(uint size)
{

	arena_destroy();

	g_arena       = malloc(size);
	g_arena_size  = size;
	g_arena_point = 0;

}



void arena_destroy()
{

	free(g_arena);

	g_arena       = NULL;
	g_arena_size  = 0;
	g_arena_point = 0;

}



void arena_reset()
{

	g_arena_point = 0;

}



void *arena_alloc(uint sz)
{

	if ((g_arena_point + sz) > g_arena_size)    // Arena boundary overrun
		abort();

	void *ptr = (u8*)g_arena + g_arena_point;

	// Move boundary and keep it aligned
	g_arena_point += (sz + (ARENA_ALIGNMENT - 1)) & -ARENA_ALIGNMENT;

	return ptr;

}



void *arena_calloc(uint n, uint sz)
{

	uint  len = n * sz;
	void *ptr = arena_alloc(len);

	return memset(ptr, 0, len);
}



uint arena_save()
{

	return g_arena_point;

}



void arena_restore(uint pt)
{

	if ((pt > g_arena_size) || (pt & (ARENA_ALIGNMENT - 1)) != 0)    // Invalid restore point
		abort();

	g_arena_point = pt;

}


