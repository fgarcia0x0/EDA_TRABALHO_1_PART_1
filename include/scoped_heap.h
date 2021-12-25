#ifndef SCOPED_HEAP_UTILS_H
#define SCOPED_HEAP_UTILS_H

#define SCOPED_HEAP_API

#include <stdint.h>
#include <stdbool.h>

#include "heap_utils.h"

#define SCOPED_HEAP_CAPACITY_FACTOR 0x2

typedef struct scoped_heap_struct
{
	uint8_t* begin;
	uint8_t* end;
	uint32_t elem_size;
	size_t capacity;
	comparator cmp_fptr;
} scoped_heap;

typedef void(*scoped_heap_action)(const uint8_t* elem_ptr);

SCOPED_HEAP_API
bool scoped_heap_traverse(scoped_heap* target, scoped_heap_action action);

SCOPED_HEAP_API
bool scoped_heap_realloc(scoped_heap** ppscpheap, uint8_t factor);

SCOPED_HEAP_API
scoped_heap* scoped_heap_create(const void* src, size_t size,
							    uint32_t elem_size, comparator cmp);

SCOPED_HEAP_API
size_t scoped_heap_size(scoped_heap* scpheap_ptr);

SCOPED_HEAP_API
bool scoped_heap_push(scoped_heap* scpheap_ptr, const void* element);

SCOPED_HEAP_API
void* scoped_heap_pop(scoped_heap* scpheap_ptr);

SCOPED_HEAP_API
void scoped_heap_release(scoped_heap** ppscpheap);

#endif
