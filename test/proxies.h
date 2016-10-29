#ifndef TEST_PROXIES_H
#define TEST_PROXIES_H


void* test_malloc(size_t size);
void test_free(void *ptr);
void test_malloc_startup();
void test_malloc_cleanup();


#endif
