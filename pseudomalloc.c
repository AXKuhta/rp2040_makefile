
#include <stddef.h>

// This file stubplements malloc(), calloc(), free() and realloc() to experiment with compilation of pico-sdk with -nostdlib
// see also pico-sdk/src/rp2_common/pico_malloc/pico_malloc.c

// Provided by the linker script
// Located at the end of bss
extern char end;

void* malloc(size_t size) {
	static size_t offset;

	char* result = &end + offset;
	offset += size;

	// Align
	offset += -offset % 4;

	return result;
}

void* calloc(size_t nmemb, size_t size) {
	char* memory = malloc(nmemb*size);

	for (size_t i = 0; i < nmemb*size; i++)
		memory[i] = 0;

	return memory;
}

void free(void* memory) {
	// We do not free
	(void)memory;
}

void* realloc(void *ptr, size_t size) {
	// Nobody calls it anyway
	(void)ptr;
	(void)size;
	while (1) {}
}
