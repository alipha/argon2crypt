#include "common.h"


int valid_memory_kbits(int memory_kbits) {
	return memory_kbits >= MIN_MEMORY_KBITS && memory_kbits <= MAX_MEMORY_KBITS;
}


int valid_iterations(int iterations) {
	return iterations >= MIN_ITERATIONS && iterations <= MAX_ITERATIONS;
}

