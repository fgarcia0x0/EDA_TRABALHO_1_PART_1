#ifndef ITER_UTILS_H
#define ITER_UTILS_H

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

static uint8_t* iter_next(void* iter, size_t n)
{
	return (uint8_t *)iter + n;
}

static uint8_t* iter_prev(void* iter, size_t n)
{
	return (uint8_t *)iter - n;
}

static void iter_swap(void* first, void* second, size_t elem_size)
{
	void* tmp = malloc(elem_size);
	if (!tmp)
		return;

	memcpy(tmp, first, elem_size);
	memmove(first, second, elem_size);
	memcpy(second, tmp, elem_size);

	free(tmp);
}

#endif
