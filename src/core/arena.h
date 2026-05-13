

#ifndef CORE_ARENA_H
#define CORE_ARENA_H


void arena_create(uint size);
void arena_destroy();

void  arena_reset();
uint  arena_free();
void *arena_alloc(uint size);
void *arena_calloc(uint n, uint size);
uint  arena_save();
void  arena_restore(uint point);


#endif


