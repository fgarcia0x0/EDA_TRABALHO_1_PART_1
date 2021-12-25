#ifndef SORT_UTILS_H
#define SORT_UTILS_H

#include "heap_utils.h"
#include "iter_utils.h"

#define POINTER_SIZE sizeof(void *)

static void heap_sort(void* begin, void* end,
			  		  size_t elem_size, comparator cmp)
{
	heap_construct(begin, end, elem_size, cmp);

	uint8_t* end_ptr = (uint8_t *)end;
	uint8_t* prev_end = iter_prev(end_ptr, elem_size);

	for (uint8_t* iter = prev_end; iter != begin; iter = iter_prev(iter, elem_size))
	{
		iter_swap(begin, iter, elem_size);
		end_ptr = iter_prev(end_ptr, elem_size);
		heap_down(begin, end_ptr, 0, elem_size, cmp);
	}
}

static void insertion_sort(void* begin, void* end,
			  		  	   size_t elem_size, comparator cmp)
{
	bool b = elem_size > POINTER_SIZE;
	static uint8_t buffer[POINTER_SIZE] = {0};
	uint8_t* key = &buffer[0];

	if (b)
	{
		key = (uint8_t *) malloc(elem_size);
		if (!key) return;
	}

	uint8_t* begin_iter = (uint8_t *) begin;
	uint8_t* end_iter = (uint8_t *) end;

	uint8_t* iter = iter_next(begin_iter, elem_size);
	uint8_t* curr = NULL;

	for (; iter != end_iter; iter = iter_next(iter, elem_size))
	{
		memcpy(key, iter, elem_size);
		curr = iter_prev(iter, elem_size);

		while((curr >= begin_iter) && cmp(key, curr, elem_size))
		{
			memcpy(iter_next(curr, elem_size), curr, elem_size);
			curr = iter_prev(curr, elem_size);
		}

		memcpy(iter_next(curr, elem_size), key, elem_size);
	}

	if (b) free(key);
}

#endif
