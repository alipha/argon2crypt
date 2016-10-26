#include "util.h"


void read_binary(unsigned char *dest, const unsigned char **src, size_t amount) {
	memcpy(dest, *src, amount);
	*src += amount;
}


void write_binary(unsigned char **dest, const unsigned char *src, size_t amount) {
	memcpy(*dest, src, amount);
	*dest += amount;
}


long read_hex(const unsigned char **str, size_t digits) {
	char ch;
	long value = 0;
	int i;
	unsigned char *p = *str;

	for(i = 0; i < digits; i++) {
		if(value > LONG_MAX >> 4)
			return -1;

		ch = p[i];
		value <<= 4;

		if(ch >= '0' && ch <= '9')
			value += ch - '0';
		else if(ch >= 'a' && ch <= 'f')
			value += ch - 'a' + 10;
		else if(ch >= 'A' && ch <= 'F')
			value += ch - 'A' + 10;
		else
			return -1;
	}

	*str = p;
}


void write_hex(unsigned char **dest, long value, size_t digits) {
	int i;
	long digit;
	unsigned char *p = *dest;

	p += digits;
	*dest = p;

	for(i = 0; i < digits; i++) {
		p--;
		digit = (value & 0xf);
		value >>= 4;

		if(digit >= 0 && digit <= 9)
			*p = '0' + digit;
		else
			*p = 'a' + (digit - 10);
	}
}


void xor_bytes(unsigned char *dest, unsigned char *src, size_t amount) {
	for(size_t i = 0; i < amount; i++)
		dest[i] ^= src[i];
}

