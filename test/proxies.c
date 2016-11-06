#include "proxies.h"
#include "../proxies.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>


typedef struct malloc_info_type {
	void *ptr;
	size_t size;
	int freed;
} malloc_info_type;


unsigned char malloc_guard[4] = {0xde, 0xad, 0xbe, 0xef};
malloc_info_type malloc_info[MALLOC_MAX_COUNT];
size_t malloc_counter = 0;


void* test_malloc(size_t size) {
	unsigned char *ptr = 0;

	if(malloc_counter >= MALLOC_MAX_COUNT) {
		fprintf(stderr, "Total number of mallocs exceeded %d\n", (int)malloc_counter);
		return 0;
	}

	if(size <= SIZE_MAX - 2 * sizeof malloc_guard)
		ptr = malloc(size + 2 * sizeof malloc_guard);
	
	if(!ptr) {
		fprintf(stderr, "Malloc #%d failed attempting to allocate %d bytes\n", (int)malloc_counter, (int)size);
		return 0;
	}

	memcpy(ptr, malloc_guard, sizeof malloc_guard);
 	ptr += sizeof malloc_guard;
	memcpy(ptr + size, malloc_guard, sizeof malloc_guard);

	malloc_info[malloc_counter].ptr = ptr;
	malloc_info[malloc_counter].size = size;
	malloc_info[malloc_counter].freed = 0;
	malloc_counter++;
	return ptr;
}


void test_free(void *ptr) {
	size_t i;
	unsigned char *p = ptr;

	if(!ptr)
		return;

	for(i = 0; i < malloc_counter; i++) {
		if(malloc_info[i].ptr == ptr)
			break;
	}

	if(i == malloc_counter) {
		fprintf(stderr, "Attempting to free a pointer that was never allocated. malloc_counter = %d\n", (int)malloc_counter);
		return;
	}

	if(malloc_info[i].freed) {
		fprintf(stderr, "malloc #%d (of %d) was already freed.\n", (int)i, (int)malloc_counter);
		return;
	}

	if(memcmp(p - sizeof malloc_guard, malloc_guard, sizeof malloc_guard) != 0)
		fprintf(stderr, "malloc prefix guard was corrupted for malloc #%d (of %d)\n", (int)i, (int)malloc_counter);

	if(memcmp(p + malloc_info[i].size, malloc_guard, sizeof malloc_guard) != 0)
		fprintf(stderr, "malloc postfix guard was corrupted for malloc #%d (of %d)\n", (int)i, (int)malloc_counter);

	malloc_info[i].freed = 1;
	//free(p - sizeof malloc_guard);
}


void test_malloc_startup(void) {
	if(malloc_counter) {
		fprintf(stderr, "test_malloc_cleanup was not called prior to test_malloc_startup.\n");
		test_malloc_cleanup();
	}
	
	p_malloc = test_malloc;
	p_free = test_free;
}


void test_malloc_cleanup(void) {
	size_t i;

	for(i = 0; i < malloc_counter; i++) {
		if(!malloc_info[i].freed) {
			fprintf(stderr, "malloc #%d (of %d) was never freed.\n", (int)i, (int)malloc_counter);
			test_free(malloc_info[i].ptr);
		}

		free((char*)malloc_info[i].ptr - sizeof malloc_guard);
	}

	p_malloc = malloc;
	p_free = free;
	malloc_counter = 0;
}
