#include "malloc.h"
#include "proxies.h"
#include "../proxies.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>


void suite_malloc(void) {
	printf("Start  suite_malloc\n");
	test_malloc_startup_and_cleanup_assign_pointers();
	test_malloc_max_count();
	test_malloc_guard();
	test_allocation_fails();
	test_free_without_malloc();
	test_free_twice();
	printf("Finish suite_malloc\n");
}


void test_malloc_startup_and_cleanup_assign_pointers(void) {
	printf("\tStart  test_malloc_startup_and_cleanup_assign_pointers\n");	
	test_malloc_startup();
	assert(p_malloc == test_malloc);
	assert(p_free == test_free);

	test_malloc_cleanup();
	assert(p_malloc == malloc);
	assert(p_free == free);
	printf("\tFinish test_malloc_startup_and_cleanup_assign_pointers\n");
}


void test_malloc_max_count(void) {
	size_t i;
	void* ptrs[MALLOC_MAX_COUNT + 2];

	printf("\tStart  test_malloc_max_count\n");
	test_malloc_startup();

	for(i = 0; i < MALLOC_MAX_COUNT; i++) {
		ptrs[i] = p_malloc(i + 5);

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

	for(i = 0; i < MALLOC_MAX_COUNT; i++) {
		ptrs[0] = p_malloc(10);
		assert(ptrs[0] != 0);
		p_free(ptrs[0]);
	}
	
	printf("\t\tExpect: error a third time\n");
	assert(p_malloc(10) == 0);

	test_malloc_cleanup();
	printf("\tFinish test_malloc_max_count\n");	
}


void test_malloc_guard(void) {
	char *p;
	char *p2;
	char *p3;
	char *p4;

	printf("\tStart  test_malloc_guard\n");
	test_malloc_startup();

	p = p_malloc(3);
	p2 = p_malloc(3);
	p3 = p_malloc(3);
	p4 = p_malloc(3);

	assert(p != 0);
	assert(p2 != 0);
	assert(p3 != 0);
	assert(p4 != 0);

	p2[-4] = 'x';
	p3[6] = 'a';
	p4[-1] = '5';
	p4[3] = '7';

	p_free(p);
	printf("\t\tExpect: #1 prefix corrupted, #2 postfix corrupted, #3 both\n");
	p_free(p2);
	p_free(p3);
	p_free(p4);
	
	test_malloc_cleanup();
	printf("\tFinish test_malloc_guard\n");
}


void test_allocation_fails(void) {
	char *p;
	char *p2;

	printf("\tStart  test_allocation_fails\n");
	test_malloc_startup();

	printf("\t\tExpect: malloc to fail twice\n");
	p = p_malloc(SIZE_MAX);
	p2 = p_malloc(SIZE_MAX - 100);

	assert(p == 0);
	assert(p2 == 0);

	test_malloc_cleanup();
	printf("\tFinish test_allocation_fails\n");
}


void test_free_without_malloc(void) {
	int test = 0;
	char *p;
	char *p2;
	char *p3;

	printf("\tStart  test_free_without_malloc\n");
	test_malloc_startup();

	p = p_malloc(10);
	p2 = p_malloc(20);

	assert(p != 0);
	assert(p2 != 0);

	p_free(p2);

	p3 = p_malloc(30);
	assert(p3 != 0);

	p_free(p3);
	p_free(0);
	printf("\t\tExpect: free a pointer that was never allocated\n");
	p_free(&test);
	printf("\t\tExpect: no error\n");
	p_free(p);

	test_malloc_cleanup();
	printf("\tFinish test_free_without_malloc\n");
}


void test_free_twice(void) {
	char *p;
	char *p2;
	char *p3;

	printf("\tStart  test_free_twice\n");
	test_malloc_startup();

	p = p_malloc(10);
	p2 = p_malloc(20);

	assert(p != 0);
	assert(p2 != 0);

	p_free(p2);

	p3 = p_malloc(30);
	assert(p3 != 0);

	p_free(p3);
	printf("\t\tExpect: malloc was already freed\n");
	p_free(p2);
	printf("\t\tExpect: no error\n");
	p_free(p);

	test_malloc_cleanup();
	printf("\tFinish test_free_twice\n");
}


void test_memory_not_freed(void) {
	char *p;
	char *p2;
	char *p3;
	char *p4;

	printf("\tStart  test_memory_not_freed\n");
	test_malloc_startup();

	p = p_malloc(10);
	p2 = p_malloc(20);

	assert(p != 0);
	assert(p2 != 0);

	p_free(p2);

	p3 = p_malloc(30);
	assert(p3 != 0);

	p_free(p3);
	printf("\t\tExpect: malloc was already freed\n");
	p_free(p2);
	printf("\t\tExpect: no error\n");

	p4 = p_malloc(40);
	assert(p4 != 0);

	printf("\t\tExpect: malloc #0 and #3 were not freed\n");

	test_malloc_cleanup();
	printf("\tFinish test_memory_not_freed\n");
}
