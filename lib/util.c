#include "util.h"
#include <string.h>
#include <limits.h>


int read_binary(unsigned char *dest, const unsigned char **src, size_t amount) {
	if(amount == 0)
		return 1;

	if(!dest || !src || !*src)
		return 0;

	memcpy(dest, *src, amount);
	*src += amount;
	return 1;
}


int write_binary(unsigned char **dest, const unsigned char *src, size_t amount) {
	if(amount == 0)
		return 1;

	if(!dest || !*dest || !src)
		return 0;

	memcpy(*dest, src, amount);
	*dest += amount;
	return 1;
}


long read_hex(const unsigned char **str, size_t digits) {
	char ch;
	long value = 0;
	size_t i;
	const unsigned char *p;

	if(digits == 0)
		return 0;

	if(!str || !*str)
		return -1;

	p = *str;

	for(i = 0; i < digits; i++) {
		if(value > LONG_MAX >> 4) {
			*str = p;
			return -1;
		}

		ch = *p;
		value <<= 4;

		if(ch >= '0' && ch <= '9') {
			value += ch - '0';
		} else if(ch >= 'a' && ch <= 'f') {
			value += ch - 'a' + 10;
		} else if(ch >= 'A' && ch <= 'F') {
			value += ch - 'A' + 10;
		} else {
			*str = p;
			return -1;
		}

		p++;
	}

	*str = p;
	return value;
}


int write_hex(unsigned char **dest, long value, size_t digits) {
	size_t i;
	long digit;
	unsigned char *p;

	if(digits == 0 && value == 0)
		return 1;

	if(!dest || !*dest || value < 0)
		return 0;

	*dest += digits;
	p = *dest;

	for(i = 0; i < digits; i++) {
		p--;
		digit = (value & 0xf);
		value >>= 4;

		if(digit >= 0 && digit <= 9)
			*p = '0' + digit;
		else
			*p = 'a' + (digit - 10);
	}

	return value == 0;
}


int xor_bytes(unsigned char *dest, const unsigned char *src, size_t amount) {
	if(amount == 0)
		return 1;

	if(!dest || !src)
		return 0;

	for(size_t i = 0; i < amount; i++)
		dest[i] ^= src[i];

	return 1;
}

