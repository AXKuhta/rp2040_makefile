
#include <stddef.h>

#include "FreeRTOS.h"

// This file stubplements malloc(), calloc(), free() and realloc() to experiment with compilation of pico-sdk with -nostdlib
// see also pico-sdk/src/rp2_common/pico_malloc/pico_malloc.c

extern char end;

// Must be called before any memory allocations
void init_memory_manager() {
	size_t heap_size = 0x20000000 + 256*1024 - (size_t)&end;

	// FreeRTOS-Kernel/portable/MemMang/heap_5.c
	HeapRegion_t xHeapRegions[] = {
		{ ( uint8_t * ) &end, heap_size },
		{ NULL, 0 }
	};

	vPortDefineHeapRegions(xHeapRegions);
}

void* malloc(size_t size) {
	return pvPortMalloc(size);
}

void* calloc(size_t nmemb, size_t size) {
	char* memory = malloc(nmemb*size);

	for (size_t i = 0; i < nmemb*size; i++)
		memory[i] = 0;

	return memory;
}

void free(void* memory) {
	vPortFree(memory);
}

void* realloc(void *ptr, size_t size) {
	// Nobody calls it anyway
	(void)ptr;
	(void)size;
	while (1) {}
}

int memcmp(const void* s1, const void* s2, size_t n) {
	const unsigned char* p1 = s1, * p2 = s2;

	while (n--) {
		if (*p1 != *p2)
			return *p1 - *p2;

		p1++;
		p2++;
	}

	return 0;
}

void* memmove(void* dest, const void* src, size_t n) {
	unsigned char* pd = dest;
	const unsigned char* ps = src;
	if (ps < pd)
		for (pd += n, ps +=n; n--;)
			*--pd = *--ps;
	else
		while (n--)
			*pd++ = *ps++;
	return dest;
}
