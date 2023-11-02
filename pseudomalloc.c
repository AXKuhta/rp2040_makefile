
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
