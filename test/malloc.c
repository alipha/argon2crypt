#include "proxies.h"
#include "../proxies.h"
#include <stdio.h>


void test_malloc_startup_and_cleanup_assign_pointers() {
	printf("\tStart  test_malloc_startup_and_cleanup_assign_pointers\n");	
	test_malloc_startup();
	assert(p_malloc == test_malloc);
	assert(p_free == test_free);

	test_malloc_cleanup();
	assert(p_malloc == malloc);
	assert(p_free == free);
	printf("\tFinish test_malloc_startup_and_cleanup_assign_pointers\n");
}


void test_malloc_max_count() {
	size_t i;
	void* ptrs[MALLOC_MAX_COUNT + 2];

	printf("\tStart  test_malloc_max_count\n");
	test_malloc_startup();

	for(i = 0; i < MALLOC_MAX_COUNT; i++) {
		ptrs[i] = p_malloc(i * 5);

		assert(ptrs[i] != 0);

		if(i == 40 || i == 55) {
			p_free(ptrs[i - 10]);
			ptrs[i - 10] = 0;
		}

		if(i == 81) {
			p_free(ptrs[i]);
			ptrs[i] = 0;
		}
	}
	
	printf("\t\tExpect: Total number of mallocs exceeded %d (twice)\n", MALLOC_MAX_COUNT);

	assert(p_malloc(500) == 0);
	assert(p_malloc(501) == 0);

	for(i = 0; i < MALLOC_MAX_COUNT; i++)
		p_free(ptrs[i]);

	test_malloc_cleanup();
	test_malloc_startup();

	printf("\t\tExpect: error a third time\n");

	for(i = 0; i < MALLOC_MAX_COUNT + 1; i++) {
		ptrs[0] = p_malloc(10);
		assert(ptrs[0] != 0);
		p_free(ptrs[0]);
	}

	test_malloc_cleanup();
	printf("\tFinish test_malloc_max_count\n");	
}


void test_malloc_guard() {
	char *p;
	char *p2;
	char *p3;
	char *p4;

	printf("\tStart  test_malloc_guard\n");
	test_malloc_startup();

	
	test_malloc_cleanup();
	printf("\tFinish test_malloc_guard\n");
}


void test_allocation_fails() {
	printf("\tStart  test_allocation_fails\n");
	test_malloc_startup();


	test_malloc_cleanup();
	printf("\tFinish test_allocation_fails\n");
}


void test_free_without_malloc() {
	printf("\tStart  test_free_without_malloc\n");
	test_malloc_startup();


	test_malloc_cleanup();
	printf("\tFinish test_free_without_malloc\n");
}


void test_free_twice() {
	printf("\tStart  test_free_twice\n");
	test_malloc_startup();


	test_malloc_cleanup();
	printf("\tFinish test_free_twice\n");
}


void test_memory_not_freed() {
	printf("\tStart  test_memory_not_freed\n");
	test_malloc_startup();


	test_malloc_cleanup();
	printf("\tFinish test_memory_not_freed\n");
}
