#ifndef COMPARE_UTILS_H
#define COMPARE_UTILS_H

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

static bool less_than(const void* left,
					  const void* right,
					  size_t size)
{
	return memcmp(left, right, size) < 0;
}

static bool less_than_i32(const void* first,
						  const void* second,
						  size_t size)
{
	(void) size;
	return *(const int *)first < *(const int *)second;
}

static bool greater_than(const void* left,
						 const void* right,
						 size_t size)
{
	return memcmp(left, right, size) > 0;
}

static bool greater_than_i32(const void* first,
							 const void* second,
							 size_t size)
{
	(void) size;
	return *(const int *)first > *(const int *)second;
}

static bool equal_than(const void* left,
					   const void* right,
					   size_t size)
{
	return memcmp(left, right, size) == 0;
}

static bool equal_than_i32(const void* first,
						   const void* second,
						   size_t size)
{
	(void) size;
	return *(const int *)first == *(const int *)second;
}

#endif
