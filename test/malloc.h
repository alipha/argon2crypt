#ifndef TEST_MALLOC_H
#define TEST_MALLOC_H

void suite_malloc(void);

void test_malloc_startup_and_cleanup_assign_pointers(void);
void test_malloc_max_count(void);
void test_malloc_guard(void);
void test_allocation_fails(void);
void test_free_without_malloc(void);
void test_free_twice(void);
void test_memory_not_freed(void);

#endif
