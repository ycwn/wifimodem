

#ifndef CORE_MEMPOOL_H
#define CORE_MEMPOOL_H


enum {
	MEMPOOL_BLOCK_SIZE = 2048,
	MEMPOOL_BLOCK_NUM  = 16
};



void  mempool_create();
void *mempool_lock();
void  mempool_unlock(void *ptr);
void  mempool_stats();

static inline void mempool_cleanup(void *p) {
	mempool_unlock(*(void**)p);
}


#define MEMBLOCK_ALLOC(type, name)                                                 \
	type *name __attribute__((__cleanup__(mempool_cleanup))) = mempool_lock(); \
	enum {                                                                     \
		name##_size  = MEMPOOL_BLOCK_SIZE,                                 \
		name##_count = name##_size / sizeof(type)                          \
	}

#define MEMBLOCK_SLICE(type, store, name, slices, start, length)                      \
	type *name = (type*)((const u8*)(store) + (start) * store##_size / (slices)); \
	enum {                                                                        \
		name##_size  = (length) * store##_size / (slices),                    \
		name##_count = name##_size / sizeof(type)                             \
	}


#endif


