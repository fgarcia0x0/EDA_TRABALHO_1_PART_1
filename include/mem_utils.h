#ifndef MEM_UTILS_H
#define MEM_UTILS_H

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>

#define ArrayCount(arr) (sizeof(arr) / sizeof(*arr))

static void* memdup(const void* src, size_t size)
{
	if (!src || !size)
		return NULL;

	void* dest = malloc(size);
	if (!dest)
		return NULL;

	return memcpy(dest, src, size);
}

static void* create_vector(size_t n, size_t elem_size, bool clear)
{
	void* mem = malloc(n * elem_size);
	if (!mem)
		return NULL;

	if (clear)
		memset(mem, 0, n * elem_size);

	return mem;
}

static inline size_t round_up_to_power_of_2(size_t n)
{
	--n;

	for (size_t i = 1u; i < sizeof(n) * CHAR_BIT; i <<= 1u)
		n |= n >> i;

	return ++n;
}

#endif
