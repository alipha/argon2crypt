#ifndef TEST_PROXIES_IO_H
#define TEST_PROXIES_IO_H

#include <stdio.h>
#include <stddef.h>


expected_call* increment_expected();
expected_call* next_expected(io_function func);


void test_fopen_add_expected(const char *path, const char *mode, FILE *return_value);
void test_fclose_add_expected(FILE *file, int return_value);
void test_fseek_add_expected(FILE *file, long offset, int whence, int return_value);
void test_ftell_add_expected(FILE *file, long return_value);

void test_fprintf_add_expected(FILE *file, const char *output, int return_error);
void test_fread_add_buffer(const void *buffer, size_t size, size_t expected_size, FILE *file);
void test_fwrite_add_expected(const void *expected, size_t size, FILE *file, int return_error);
void test_fgets_add_buffer(const char *buffer, int size, int expected_size, FILE *file);
void test_fgetc_add_buffer(const char *buffer, int size, FILE *file);

void test_io_startup(void);
void test_io_cleanup(void);


FILE* test_fopen(const char *path, const char *mode);
int test_fclose(FILE *file);
int test_fseek(FILE *file, long offset, int whence);
long test_ftell(FILE *file);

int test_fprintf(FILE *file, const char *format, ...);
size_t test_fread(void *ptr, size_t size, size_t count, FILE *file);
size_t test_fwrite(const void *ptr, size_t size, size_t count, FILE *file);
char* test_fgets(char *s, int size, FILE *file);
int test_fgetc(FILE *file);

//int (*test_fileno)(FILE *file);
void test_perror(const char *str);


#endif
