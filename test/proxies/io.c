#include "io.h"
#include "../../lib/proxies.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stddef.h>

#ifndef OVERRIDE_ASSERT
#include <assert.h>
#else
#define assert(x) if(!(x)) printf("Assert failed on line %d: %s", __LINE__, "#x")
#endif

#define MAX_EXPECTED_CALLS 100
#define FPRINTF_BUF_SIZE 10000


typedef struct {
	const char *path;
	const char *mode;
	FILE *return_value;
} fopen_arguments;

typedef struct {
	FILE *file;
	int return_value;
} fclose_arguments;

typedef struct {
	FILE *file;
	long offset;
	int whence;
	int return_value;
} fseek_arguments;

typedef struct {
	FILE *file;
	long return_value;
} ftell_arguments;

typedef struct {
	FILE *file;
	const char *expected_output;
	int return_error;
} fprintf_arguments;

typedef struct {
	const void *buffer;
	size_t buffer_size;
	size_t expected_byte_count;
	FILE *file;
} fread_arguments;

typedef struct {
	const void *expected;
	size_t size;
	FILE *file;
	size_t return_value;
} fwrite_arguments;

typedef struct {
	const char *buffer;
	int expected_size;
	FILE *file;
} fgets_arguments;

typedef struct {
	const char *buffer;
	int size;
	FILE *file;
} fgetc_arguments;


typedef struct expected_call {
	io_function function;
	union {
		fopen_arguments fopen_args;
		fclose_arguments fclose_args;
		fseek_arguments fseek_args;
		ftell_arguments ftell_args;
		fprintf_arguments fprintf_args;
		fread_arguments fread_args;
		fwrite_arguments fwrite_args;
		fgets_arguments fgets_args;
		fgetc_arguments fgetc_args;
	} args;
} expected_call;



expected_call expected_calls[MAX_EXPECTED_CALLS];
char fprintf_buffer[FPRINTF_BUF_SIZE];

int expected_call_count;
int expected_call_index;



struct expected_call* increment_expected(io_function func) {
	expected_call *call = expected_calls + expected_call_count;

	expected_call_count++;
	assert(expected_call_count <= MAX_EXPECTED_CALLS);

	call->function = func;
	return call;
}


struct expected_call* next_expected(io_function func) {
	expected_call *call = expected_calls + expected_call_index;

	assert(expected_call_index < expected_call_count);
	assert(call->function == func);

	expected_call_index++;
	return call;
}



void test_fopen_add_expected(const char *path, const char *mode, FILE *return_value) {
	expected_call *call = increment_expected(IO_FOPEN);

	assert(path != NULL);
	assert(mode != NULL);

	call->args.fopen_args.path = path;
	call->args.fopen_args.mode = mode;
	call->args.fopen_args.return_value = return_value;
}


void test_fclose_add_expected(FILE *file, int return_value) {
	expected_call *call = increment_expected(IO_FCLOSE);

	assert(file != NULL);

	call->args.fclose_args.file = file;
	call->args.fclose_args.return_value = return_value;
}


void test_fseek_add_expected(FILE *file, long offset, int whence, int return_value) {
	expected_call *call = increment_expected(IO_FSEEK);

	assert(file != NULL);
	assert(whence == SEEK_SET || whence == SEEK_CUR || whence == SEEK_END);

	call->args.fseek_args.file = file;
	call->args.fseek_args.offset = offset;
	call->args.fseek_args.whence = whence;
	call->args.fseek_args.return_value = return_value;
}


void test_ftell_add_expected(FILE *file, long return_value) {
	expected_call *call = increment_expected(IO_FTELL);

	assert(file != NULL);

	call->args.ftell_args.file = file;
	call->args.ftell_args.return_value = return_value;
}


void test_fprintf_add_expected(FILE *file, const char *expected_output, int return_error) {
	expected_call *call = increment_expected(IO_FPRINTF);

	assert(file != NULL);
	assert(expected_output != NULL);

	call->args.fprintf_args.file = file;
	call->args.fprintf_args.expected_output = expected_output;
	call->args.fprintf_args.return_error = return_error;
}


void test_fread_add_buffer(const void *buffer, size_t buffer_size, size_t expected_byte_count, FILE *file) {
	expected_call *call = increment_expected(IO_FREAD);

	assert(file != NULL);
	assert(buffer != NULL || buffer_size == 0);

	call->args.fread_args.buffer = buffer;
	call->args.fread_args.buffer_size = buffer_size;
	call->args.fread_args.expected_byte_count = expected_byte_count;
	call->args.fread_args.file = file;
}


void test_fwrite_add_expected(const void *expected, size_t size, FILE *file, size_t return_value) {
	expected_call *call = increment_expected(IO_FWRITE);

	assert(file != NULL);
	assert(expected != NULL || size == 0);

	call->args.fwrite_args.expected = expected;
	call->args.fwrite_args.size = size;
	call->args.fwrite_args.file = file;
	call->args.fwrite_args.return_value = return_value;
}


void test_fgets_add_buffer(const char *buffer, int expected_size, FILE *file) {
	expected_call *call = increment_expected(IO_FGETS);

	assert(file != NULL);
	assert(expected_size >= 0);

	call->args.fgets_args.buffer = buffer;
	call->args.fgets_args.expected_size = expected_size;
	call->args.fgets_args.file = file;
}


void test_fgetc_add_buffer(const char *buffer, int size, FILE *file) {
	expected_call *call = increment_expected(IO_FGETC);

	assert(file != NULL);
	assert(buffer != NULL || size == 0);
	assert(size >= 0);

	call->args.fgetc_args.buffer = buffer;
	call->args.fgetc_args.size = size;
	call->args.fgetc_args.file = file;
}



void test_io_startup(void) {
	expected_call_count = expected_call_index = 0;

	p_fopen = test_fopen;
	p_fclose = test_fclose;
	p_fseek = test_fseek;
	p_ftell = test_ftell;

	p_fprintf = test_fprintf;
	p_fread = test_fread;
	p_fwrite = test_fwrite;
	p_fgets = test_fgets;
	p_fgetc = test_fgetc;
}


void test_io_cleanup(void) {
	assert(expected_call_index == expected_call_count);

	p_fopen = fopen;
	p_fclose = fclose;
	p_fseek = fseek;
	p_ftell = ftell;

	p_fprintf = fprintf;
	p_fread = fread;
	p_fwrite = fwrite;
	p_fgets = fgets;
	p_fgetc = fgetc;
}



FILE* test_fopen(const char *path, const char *mode) {
	expected_call *call = next_expected(IO_FOPEN);

	assert(path != NULL);
	assert(mode != NULL);
	assert(strcmp(path, call->args.fopen_args.path) == 0);
	assert(strcmp(mode, call->args.fopen_args.mode) == 0);

	return call->args.fopen_args.return_value;
}


int test_fclose(FILE *file) {
	expected_call *call = next_expected(IO_FCLOSE);

	assert(file == call->args.fclose_args.file);
	
	return call->args.fclose_args.return_value;
}


int test_fseek(FILE *file, long offset, int whence) {
	expected_call *call = next_expected(IO_FSEEK);

	assert(file == call->args.fseek_args.file);
	assert(offset == call->args.fseek_args.offset);
	assert(whence == call->args.fseek_args.whence);

	return call->args.fseek_args.return_value;
}


long test_ftell(FILE *file) {
	expected_call *call = next_expected(IO_FTELL);

	assert(file == call->args.ftell_args.file);

	return call->args.ftell_args.return_value;
}


int test_fprintf(FILE *file, const char *format, ...) {
	va_list va;
	expected_call *call = next_expected(IO_FPRINTF);
	
	assert(file == call->args.fprintf_args.file);

	va_start(va, format);
	vsnprintf(fprintf_buffer, FPRINTF_BUF_SIZE, format, va);
	va_end(va);

	assert(strcmp(fprintf_buffer, call->args.fprintf_args.expected_output) == 0);

	if(call->args.fprintf_args.return_error)
		return -1;
	else
		return strlen(fprintf_buffer);
}


size_t test_fread(void *ptr, size_t size, size_t count, FILE *file) {
	expected_call *call = next_expected(IO_FREAD);
	size_t actual_byte_count = size * count;
	size_t buffer_size = call->args.fread_args.buffer_size;

	assert(ptr != NULL);
	assert(file == call->args.fread_args.file);
	assert(actual_byte_count == call->args.fread_args.expected_byte_count);
	assert(buffer_size % size == 0);
	assert(buffer_size <= actual_byte_count);

	memcpy(ptr, call->args.fread_args.buffer, buffer_size);
	return buffer_size / size;	
}


size_t test_fwrite(const void *ptr, size_t size, size_t count, FILE *file) {
	expected_call *call = next_expected(IO_FWRITE);
	size_t byte_count = size * count;

	assert(ptr != NULL);
	assert(file == call->args.fwrite_args.file);
	assert(byte_count == call->args.fwrite_args.size);
	assert(memcmp(ptr, call->args.fwrite_args.expected, byte_count) == 0);

	return call->args.fwrite_args.return_value;
}


char* test_fgets(char *s, int size, FILE *file) {
	expected_call *call = next_expected(IO_FGETS);
	const char *buffer = call->args.fgets_args.buffer;
	size_t len = 0;

	if(buffer)
		len = strlen(buffer);

	assert(s != NULL);
	assert(file == call->args.fgets_args.file);
	assert(size > 1);
	assert(size == call->args.fgets_args.expected_size);
	assert(len < size);
	
	if(buffer)
		strcpy(s, buffer);
	else
		s[0] = '\0';

	if(len == 0)
		return NULL;
	else
		return s;
}


int test_fgetc(FILE *file) {
	expected_call *call = next_expected(IO_FGETC);
	int size = call->args.fgetc_args.size; 
	const char *buffer = call->args.fgetc_args.buffer;

	assert(file == call->args.fgetc_args.file);

	if(size == 0)
		return EOF;

	call->args.fgetc_args.size--;
	call->args.fgetc_args.buffer++;

	// reset index to point to the same IO_FGETC because we still
	// have more chars to read
	if(size > 1)
		expected_call_index--;

	return *buffer;
}


//int (*test_fileno)(FILE *file) {
void test_perror(const char *str) {

}

