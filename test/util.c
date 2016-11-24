#include "util.h"
#include "../lib/util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <assert.h>


void suite_util(void) {
	printf("Start  suite_util\n");
	test_read_binary();
	test_write_binary();
	test_read_hex();
	test_write_hex();
	test_xor_bytes();
	printf("Finish suite_util\n");
}


void test_read_binary(void) {
	unsigned char original_src[] = {10, 11, 12, 13, 14, 15, 16};
	unsigned char src[] = {10, 11, 12, 13, 14, 15, 16};
	unsigned char dest[] = {20, 21, 22, 23, 24};
	unsigned char dest2[] = {30, 31, 32, 33};
	unsigned char expected[] = {20, 11, 12, 13, 24};
	unsigned char expected2[] = {30, 14, 15, 33};
	const unsigned char *p = src + 1;
	const unsigned char *null_ptr = NULL;

	printf("\tStart  test_read_binary\n");
	
	assert(read_binary(NULL, &p, 0));
	assert(read_binary(NULL, NULL, 0));
	assert(read_binary(dest + 1, NULL, 0));
	assert(read_binary(dest + 1, &null_ptr, 0));
	assert(read_binary(NULL, &null_ptr, 0));
	assert(!read_binary(NULL, &p, 1));
	assert(!read_binary(NULL, NULL, 5));
	assert(!read_binary(dest + 1, NULL, 90));
	assert(!read_binary(dest + 1, &null_ptr, 3));
	assert(!read_binary(NULL, &null_ptr, 1));
	
	assert(dest[1] == 21);

	assert(*p == 11);
	assert(read_binary(dest + 1, &p, 3));
	assert(*p == 14);

	assert(read_binary(dest2 + 1, &p, 2));
	assert(*p == 16);
	assert(read_binary(dest2, &p, 0));
	assert(*p == 16);

	assert(memcmp(src, original_src, sizeof src) == 0);
	assert(memcmp(dest, expected, sizeof dest) == 0);
	assert(memcmp(dest2, expected2, sizeof dest2) == 0);

	printf("\tFinish test_read_binary\n");
}


void test_write_binary(void) {
	unsigned char original_src[] = {20, 21, 22, 23, 24};
	unsigned char original_src2[] = {30, 31, 32, 33};
	unsigned char src[] = {20, 21, 22, 23, 24};
	unsigned char src2[] = {30, 31, 32, 33};
	unsigned char dest[] = {10, 11, 12, 13, 14, 15, 16};
	unsigned char expected[] = {10, 21, 22, 23, 31, 32, 16};
	unsigned char *p = dest + 1;
	unsigned char *null_ptr = NULL;

	printf("\tStart  test_write_binary\n");

	assert(write_binary(&p, NULL, 0));
	assert(write_binary(NULL, NULL, 0));
	assert(write_binary(NULL, src + 1, 0));
	assert(write_binary(&null_ptr, src + 1, 0));
	assert(write_binary(&null_ptr, NULL, 0));
	assert(!write_binary(&p, NULL, 53));
	assert(!write_binary(NULL, NULL, 1));
	assert(!write_binary(NULL, src + 1, 23));
	assert(!write_binary(&null_ptr, src + 1, 1));
	assert(!write_binary(&null_ptr, NULL, 6));

	assert(*p == 11);
	assert(write_binary(&p, src + 1, 3));
	assert(*p == 14);

	assert(write_binary(&p, src2 + 1, 2));
	assert(*p == 16);
	assert(write_binary(&p, src2, 0));
	assert(*p == 16);

	assert(memcmp(src, original_src, sizeof src) == 0);
	assert(memcmp(src2, original_src2, sizeof src2) == 0);
	assert(memcmp(dest, expected, sizeof dest) == 0);

	printf("\tFinish test_write_binary\n");
}


void test_read_hex(void) {
	size_t i;
	char src1[] = "32Fa84x";
	char src2[sizeof(long) * 2 + 1] = "7";
	char src3[sizeof src2] = "8";

	const unsigned char *p1 = (unsigned char *)src1;
	const unsigned char *p2 = (unsigned char *)src2;
	const unsigned char *p3 = (unsigned char *)src3;
	const unsigned char *null_ptr = NULL;

	printf("\tStart  test_read_hex\n");

	for(i = 1; i < sizeof src2 - 1; i++) {
		src2[i] = 'F';
		src3[i] = '0';
	}

	assert(read_hex(NULL, 0) == 0);
	assert(read_hex(NULL, 1) == -1);
	assert(read_hex(&null_ptr, 0) == 0);
	assert(read_hex(&null_ptr, 5) == -1);

	assert(read_hex(&p1, 4) == 0x32fa);
	assert((char)*p1 == '8');

	assert(read_hex(&p1, 1) == 8);
	assert((char)*p1 == '4');

	assert(read_hex(&p1, 0) == 0);
	assert((char)*p1 == '4');

	assert(read_hex(&p1, 2) == -1);
	assert((char)*p1 == 'x');

	assert(read_hex(&p2, sizeof src2 - 1) == LONG_MAX);
	assert((char)*p2 == '\0');

	assert(read_hex(&p3, sizeof src3 - 1) == -1);
	assert((char)*p3 == '0');

	printf("\tFinish test_read_hex\n");
}


void test_write_hex(void) {
	char dest[] = "xmnopqrstz";
	char expected[] = "xa1f009z";
	unsigned char *p = (unsigned char *)dest + 1;
	unsigned char *null_ptr = NULL;

	printf("\tStart  test_write_hex\n");

	assert(write_hex(NULL, 0, 0));
	assert(write_hex(&null_ptr, 0, 0));
	assert(!write_hex(NULL, 3, 0));
	assert(!write_hex(&null_ptr, 0, 2));
	assert(!write_hex(NULL, 99, 5));
	assert(!write_hex(&null_ptr, 50, 8));

	assert(write_hex(&p, 0xa1f, 3));
	assert((char)*p == 'p');

	assert(write_hex(&p, 9, 3));	
	assert((char)*p == 's');

	assert(!write_hex(&p, -55, 2));
	assert((char)*p == 's');

	assert(write_hex(&p, 0, 0));
	assert((char)*p == 's');

	assert(!write_hex(&p, 0x54321, 2));
	assert((char)*p == 'z');

	printf("\tFinish test_write_hex\n");
}


void test_xor_bytes(void) {
	unsigned char dest[] = {15, 3, 9, 2, 17};
	unsigned char src[] = {12, 5, 1, 8, 12, 19};
	unsigned char expected_dest[] = {15, 2, 1, 14, 17};
	unsigned char expected_src[] = {12, 5, 1, 8, 12, 19};

	printf("\tStart  test_xor_bytes\n");

	assert(xor_bytes(NULL, src, 0));
	assert(xor_bytes(dest, NULL, 0));
	assert(xor_bytes(NULL, NULL, 0));
	assert(!xor_bytes(NULL, src, 5));
	assert(!xor_bytes(dest, NULL, 1));
	assert(!xor_bytes(NULL, NULL, 10));

	assert(xor_bytes(dest + 1, src + 2, 3));
	assert(memcmp(dest, expected_dest, sizeof dest) == 0);
	assert(memcmp(src, expected_src, sizeof src) == 0);
	assert(xor_bytes(dest, src, 0));
	assert(memcmp(dest, expected_dest, sizeof dest) == 0);
	assert(memcmp(src, expected_src, sizeof src) == 0);

	printf("\tFinish test_xor_bytes\n");
}

