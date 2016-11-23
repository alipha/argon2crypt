#include "../lib/util.h"
#include <string.h>
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
	unsigned char *p = src + 1;

	printf("\tStart  test_read_binary\n");
	
	assert(*p == 11);
	read_binary(dest + 1, &p, 3);
	assert(*p == 14);

	read_binary(dest2 + 1, &p, 2);
	assert(*p == 16);
	read_binary(dest2, &p, 0);
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

	printf("\tStart  test_write_binary\n");

	assert(*p == 11);
	write_binary(&p, src + 1, 3);
	assert(*p == 14);

	write_binary(&p, src2 + 1, 2);
	assert(*p == 16);
	write_binary(&p, src2, 0);
	assert(*p == 16);

	assert(memcmp(src, original_src, sizeof src) == 0);
	assert(memcmp(src2, original_src2, sizeof src2) == 0);
	assert(memcmp(dest, expected, sizeof dest) == 0);

	printf("\tFinish test_write_binary\n");
}


void test_read_hex(void) {
	char src1[] = "32Fa84x";
	char src2[] = "7FFFFFFF";
	char src3[] = "80000000";

	unsigned char *p1 = (unsigned char *)src1;
	unsigned char *p2 = (unsigned char *)src2;
	unsigned char *p3 = (unsigned char *)src3;

	long dest1a;
	long dest1b;
	long dest1c;
	long dest1d;

	printf("\tStart  test_read_hex\n");

	dest1a = read_hex(&p1, 4);
	assert((char)*p1 == '8');

	dest1b = read_hex(&p1, 1);
	assert((char)*p1 == '4');

	dest1c = read_hex(&p1, 0);
	assert((char)*p1 == '4');

	dest1d = read_hex(&p1, 2);
	assert((char)*p1 == '4');

	dest2 = read_hex(&p2, 8);
	assert((char)*p2 == '\0');

	dest2 = read_hex(&p3, 8);
	assert((char)*p2 == '8');

	assert(dest1a == 0x32fa);
	assert(dest1b == 8);
	assert(dest1c == 0);
	assert(dest1d == -1);
	assert(dest2 == 0x7fffffff);
	assert(dest3 == -1);

	printf("\tFinish test_read_hex\n");
}


void test_write_hex(void) {
	char dest[] = "xmnopz";

	printf("\tStart  test_write_hex\n");

	
	printf("\tFinish test_write_hex\n");
}


void test_xor_bytes(void) {
	unsigned char dest[] = {15, 3, 9, 2, 17};
	unsigned char src[] = {12, 5, 1, 8, 12, 19};
	unsigned char expected_dest[] = {15, 2, 1, 14, 17};
	unsigned char expected_src[] = {12, 5, 1, 8, 12, 19};

	printf("\tStart  test_xor_bytes\n");

	xor_bytes(dest + 1, src + 2, 3);
	assert(memcmp(dest, expected_dest, sizeof dest) == 0);
	assert(memcmp(src, expected_src, sizeof src) == 0);
	xor_bytes(dest, src, 0);
	assert(memcmp(dest, expected_dest, sizeof dest) == 0);
	assert(memcmp(src, expected_src, sizeof src) == 0);

	printf("\tFinish test_xor_bytes\n");
}

