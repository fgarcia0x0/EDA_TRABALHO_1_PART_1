#include "../include/utils.h"
#include "../include/scoped_heap.h"

bool scoped_heap_realloc(scoped_heap** ppscpheap, uint8_t factor)
{
	if (!ppscpheap || !*ppscpheap || !factor)
		return false;

	scoped_heap* scpheap_ptr = *ppscpheap;

	size_t size = scoped_heap_size(scpheap_ptr);
	size_t old_capacity = scpheap_ptr->capacity * scpheap_ptr->elem_size;
	size_t new_capacity = factor * old_capacity;

	uint8_t* new_begin = (uint8_t *) malloc(new_capacity);
	if (!new_begin)
		return false;

	uint8_t* dest = new_begin;
	size_t dest_size = new_capacity;

	if (scpheap_ptr->begin)
	{
		memcpy(dest, scpheap_ptr->begin, old_capacity);
		dest += old_capacity;
		dest_size = old_capacity;
		free(scpheap_ptr->begin);
	}

	memset(dest, 0, dest_size);

	scpheap_ptr->begin = new_begin;
	scpheap_ptr->end = new_begin + (size * scpheap_ptr->elem_size);
	scpheap_ptr->capacity = new_capacity / scpheap_ptr->elem_size;

	return true;
}

scoped_heap* scoped_heap_create(const void* src, size_t size,
								uint32_t elem_size, comparator cmp)
{
	if (!elem_size || !cmp)
		return NULL;

	scoped_heap* heap = (scoped_heap *) memdup(&(scoped_heap){
															   .elem_size = elem_size,
															   .cmp_fptr = cmp
															 },
															 sizeof(scoped_heap));
	if (!heap)
		return NULL;

	size_t capacity = heap->capacity = 1u;

	if (src && size)
		capacity *= size;

	if (!scoped_heap_realloc(&heap, SCOPED_HEAP_CAPACITY_FACTOR))
	{
		free(heap);
		return NULL;
	}

	if (src)
	{
		memcpy(heap->begin, src, size * elem_size);
		heap->end = heap->begin + (size * elem_size);
		heap_construct(heap->begin, heap->end, elem_size, cmp);
	}

	return heap;
}

bool scoped_heap_traverse(scoped_heap* target, scoped_heap_action action)
{
	if (!target || !action)
		return false;

	for (uint8_t* ptr = target->begin; ptr != target->end; ptr += target->elem_size)
		action(ptr);

	return true;
}

size_t scoped_heap_size(scoped_heap* scpheap_ptr)
{
	if (!scpheap_ptr || !scpheap_ptr->begin)
		return 0u;
	else
		return (size_t)(scpheap_ptr->end - scpheap_ptr->begin) / scpheap_ptr->elem_size;
}

bool scoped_heap_push(scoped_heap* scpheap_ptr, const void* element)
{
	if (!scpheap_ptr || !element)
		return false;

	size_t size = scoped_heap_size(scpheap_ptr);

	if (size >= scpheap_ptr->capacity)
		if (!scoped_heap_realloc(&scpheap_ptr, SCOPED_HEAP_CAPACITY_FACTOR))
			return false;

	size_t offset = (size * scpheap_ptr->elem_size);
	uint8_t* insertion_point = scpheap_ptr->begin + offset;

	memcpy(insertion_point, element, scpheap_ptr->elem_size);
	scpheap_ptr->end += scpheap_ptr->elem_size;

	// ajust heap tree
	heap_push(scpheap_ptr->begin, scpheap_ptr->end,
			  scpheap_ptr->elem_size, scpheap_ptr->cmp_fptr);

	return true;
}

void* scoped_heap_pop(scoped_heap* scpheap_ptr)
{
	if (!scpheap_ptr || !scpheap_ptr->begin)
		return NULL;

	size_t size = scoped_heap_size(scpheap_ptr);
	size_t offset = scpheap_ptr->elem_size;
	void* element = NULL;

	switch (size)
	{
		case 0:
			return NULL;
		case 1:
			element = scpheap_ptr->begin;
			break;
		default:
			heap_pop(scpheap_ptr->begin, scpheap_ptr->end,
				     scpheap_ptr->elem_size, scpheap_ptr->cmp_fptr);
			element = scpheap_ptr->end - offset;
			break;
	}

	scpheap_ptr->end -= offset;
	return element;
}

void scoped_heap_release(scoped_heap** ppscpheap)
{
	if (!ppscpheap || !*ppscpheap)
		return;

	free((*ppscpheap)->begin);
	free(*ppscpheap);

	*ppscpheap = NULL;
}
