#ifndef TEST_PROXIES_H
#define TEST_PROXIES_H

#include <stddef.h>

#define MALLOC_MAX_COUNT 100


extern unsigned char malloc_guard[4];


void* test_malloc(size_t size);
void test_free(void *ptr);
void test_malloc_startup(void);
void test_malloc_cleanup(void);


#endif
