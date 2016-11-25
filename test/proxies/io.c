#include "io.h"
#include <stdio.h>
#include <stddef.h>

#define MAX_EXPECTED_CALLS 100


typedef enum {
	IO_NONE,
	IO_FOPEN,
	IO_FCLOSE,
	IO_FSEEK,
	IO_FTELL,
	IO_FPRINTF,
	IO_FREAD,
	IO_FWRITE,
	IO_FGETS,
	IO_FGETC
} io_function;


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
	size_t size;
	size_t expected_size;
	FILE *file;
} fread_arguments;

typedef struct {
	const void *expected;
	size_t size;
	FILE *file;
	int return_error;
} fwrite_arguments;

typedef struct {
	const char *buffer;
	int size;
	int expected_size;
	FILE *file;
} fgets_arguments;

typedef struct {
	const char *buffer;
	int size;
	FILE *file;
} fgetc_arguments;


typedef struct {
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

int expected_call_count;
int expected_call_index;



expected_call* increment_expected(io_function func) {
	expected_call *call = expected_calls + expected_call_count;

	expected_call_count++;
	assert(expected_call_count <= MAX_EXPECTED_CALLS);

	call->function = func;
	return call;
}


expected_call* next_expected(io_function func) {
	expected_call *call = expected_calls + expected_call_index;

	assert(expected_call_index < expected_call_count);
	assert(call->function == func);

	expected_call_index++;
	return call;
}



void test_fopen_add_expected(const char *path, const char *mode, FILE *return_value) {
	expected_call *call = increment_expected(IO_FOPEN);
	call->fopen_args.path = path;
	call->fopen_args.mode = mode;
	call->fopen_args.return_value = return_value;
}


void test_fclose_add_expected(FILE *file, int return_value) {
	expected_call *call = increment_expected(IO_FCLOSE);
	call->fclose_args.file = file;
	call->fclose_args.return_value = return_value;
}


void test_fseek_add_expected(FILE *file, long offset, int whence, int return_value) {
	expected_call *call = increment_expected(IO_FSEEK);
	call->fseek_args.file = file;
	call->fseek_args.offset = offset;
	call->fseek_args.whence = whence;
	call->fseek_args.return_value = return_value;
}


void test_ftell_add_expected(FILE *file, long return_value) {
	expected_call *call = increment_expected(IO_FTELL);
	call->ftell_args.file = file;
	call->ftell_args.return_value = return_value;
}


void test_fprintf_add_expected(FILE *file, const char *output, int return_error) {
	expected_call *call = increment_expected(IO_FPRINTF);
	call->fprintf_args.file = file;
	call->fprintf_args.output = output;
	call->fprintf_args.return_error = return_error;
}


void test_fread_add_buffer(const void *buffer, size_t size, size_t expected_size, FILE *file) {
	expected_call *call = increment_expected(IO_FREAD);
	call->fread_args. = ;
	call->fread_args. = ;
	call->fread_args. = ;
	call->fread_args. = ;
}


void test_fwrite_add_expected(const void *expected, size_t size, FILE *file, int return_error) {
	expected_call *call = increment_expected(IO_FWRITE);
	call->fwrite_args. = ;
	call->fwrite_args. = ;
	call->fwrite_args. = ;
	call->fwrite_args. = ;
}


void test_fgets_add_buffer(const char *buffer, int size, int expected_size, FILE *file) {
	expected_call *call = increment_expected(IO_FGETS);
	call->fgets_args. = ;
	call->fgets_args. = ;
	call->fgets_args. = ;
	call->fgets_args. = ;
}


void test_fgetc_add_buffer(const char *buffer, int size, FILE *file) {
	expected_call *call = increment_expected(IO_FGETC);
	call->fgetc_args. = ;
	call->fgetc_args. = ;
	call->fgetc_args. = ;
}



void test_io_startup(void) {
}


void test_io_cleanup(void) {
}



FILE* test_fopen(const char *path, const char *mode) {
	expected_call *call = next_expected(IO_FOPEN);

	assert(path != NULL);
	assert(mode != NULL);
	assert(strcmp(path, call->fopen_args.path) == 0);
	assert(strcmp(mode, call->fopen_args.mode) == 0);

	return call->fopen_args.return_value;
}


int test_fclose(FILE *file) {
	expected_call *call = next_expected(IO_FCLOSE);

	assert(file == call->fclose_args.file);
	
	return call->fclose_args.return_value;
}


int test_fseek(FILE *file, long offset, int whence) {
	expected_call *call = next_expected(IO_FSEEK);

	assert(file == call->fseek_args.file);
	assert(offset == call->fseek_args.offset);
	assert(whence == call->fseek_args.whence);

	return call->fseek_args.return_value;
}


long test_ftell(FILE *file) {
	expected_call *call = next_expected(IO_FTELL);

	assert(file == call->ftell_args.file);

	return call->ftell_args.return_value;
}


int test_fprintf(FILE *file, const char *format, ...) {

}

size_t test_fread(void *ptr, size_t size, size_t count, FILE *file) {

}

size_t test_fwrite(const void *ptr, size_t size, size_t count, FILE *file) {

}

char* test_fgets(char *s, int size, FILE *file) {

}

int test_fgetc(FILE *file) {

}


//int (*test_fileno)(FILE *file) {
void test_perror(const char *str) {

}

