#ifndef HEAP_UTILS_H
#define HEAP_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "iter_utils.h"

#define HEAP_API

typedef bool(*comparator)(const void* left, const void* right, size_t elem_size);

static HEAP_API size_t heap_parent(size_t index)
{
	return (index - 1) >> 1;
}

static HEAP_API size_t heap_left(size_t index)
{
	return (index << 1) + 1;
}

static HEAP_API size_t heap_right(size_t index)
{
	return (index << 1) + 2;
}

static HEAP_API void heap_upper(void* begin, void* end,
								size_t index, size_t elem_size,
								comparator cmp)
{
	size_t len = (size_t)((uint8_t *)end - (uint8_t *)begin) / elem_size;
	if (index >= len)
	{
		fprintf(stderr, "[EXCEPTION] index must be less than vector len\n");
		return;
	}

	size_t index_parent = heap_parent(index);

	uint8_t* first  = (uint8_t *)begin + (index * elem_size);
	uint8_t* second = (uint8_t *)begin + (index_parent * elem_size);

	while (index && cmp(first, second, elem_size))
	{
		iter_swap(first, second, elem_size);

		index = index_parent;
		index_parent = heap_parent(index);

		first  = (uint8_t *)begin + (index * elem_size);
		second = (uint8_t *)begin + (index_parent * elem_size);
	}
}

static HEAP_API void heap_down(void* begin, void* end,
							   size_t index, size_t elem_size,
							   comparator cmp)
{
	size_t n = (size_t)((uint8_t *)end - (uint8_t *)begin) / elem_size;

	if (index >= n)
	{
		fprintf(stderr, "[EXCEPTION] index must be less than vector len\n");
		return;
	}

	size_t lchild = heap_left(index);
	size_t rchild = heap_right(index);
	size_t largest = index;

	while (index < n - 1)
	{
		if (lchild < n && cmp((uint8_t *)begin + (lchild * elem_size),
							  (uint8_t *)begin + (index * elem_size),
							  elem_size))
		{
			largest = lchild;
		}

		if (rchild < n && cmp((uint8_t *)begin + (rchild * elem_size),
						      (uint8_t *)begin + (largest * elem_size),
							  elem_size))
		{
			largest = rchild;
		}

		if (largest == index)
			break;

		iter_swap((uint8_t *)begin + (index * elem_size),
				 (uint8_t *)begin + (largest * elem_size), elem_size);

		index = largest;
		lchild = heap_left(index);
		rchild = heap_right(index);
	}
}

static HEAP_API void heap_construct(void* begin, void* end,
				    		 		size_t elem_size, comparator cmp)
{
	size_t n = (size_t)((uint8_t *)end - (uint8_t *)begin) / elem_size;

	for (size_t index = n >> 1; index; --index)
		heap_down(begin, end, index, elem_size, cmp);

	heap_down(begin, end, 0, elem_size, cmp);
}

static HEAP_API void heap_push(void* begin, void* end,
			   				   size_t elem_size, comparator cmp)
{
	size_t n = (size_t)((uint8_t *)end - (uint8_t *)begin) / elem_size;
	heap_upper(begin, end, n - 1, elem_size, cmp);
}

static HEAP_API void heap_pop(void* begin, void* end,
			  		  		  size_t elem_size, comparator cmp)
{
	uint8_t* prev_end = (uint8_t *)end - (1 * elem_size);
	iter_swap(begin, prev_end, elem_size);
	heap_down(begin, prev_end, 0, elem_size, cmp);
}

#endif
