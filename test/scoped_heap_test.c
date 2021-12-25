#include <stdio.h>

#include "../include/utils.h"
#include "../include/scoped_heap.h"

int main(int argc, char** argv)
{
	scoped_heap* maxheap = scoped_heap_create(NULL, 0, sizeof(int), greater_than_i32);
	if (!maxheap)
		return EXIT_FAILURE;

	int correct_result[] = { 1000, 10, 35, 3, 4, 6 };

	bool h0 = scoped_heap_push(maxheap, &(int){4});
	printf("\nsize: %zu | capacity: %zu\n", scoped_heap_size(maxheap), maxheap->capacity);

	bool h1 = scoped_heap_push(maxheap, &(int){6});
	printf("size: %zu | capacity: %zu\n", scoped_heap_size(maxheap), maxheap->capacity);

	bool h2 = scoped_heap_push(maxheap, &(int){10});
	printf("size: %zu | capacity: %zu\n", scoped_heap_size(maxheap), maxheap->capacity);

	bool h3 = scoped_heap_push(maxheap, &(int){3});
	printf("size: %zu | capacity: %zu\n", scoped_heap_size(maxheap), maxheap->capacity);

	bool h4 = scoped_heap_push(maxheap, &(int){1000});
	printf("size: %zu | capacity: %zu\n", scoped_heap_size(maxheap), maxheap->capacity);

	bool h5 = scoped_heap_push(maxheap, &(int){35});
	printf("size: %zu | capacity: %zu\n", scoped_heap_size(maxheap), maxheap->capacity);

	putchar('\n');

	if (h0 && h1 && h2 && h3 && h4 && h5)
	{
		size_t size = ArrayCount(correct_result) * sizeof(*correct_result);
		if (!memcmp(maxheap->begin, correct_result, size))
		{
			scoped_heap_release(&maxheap);
			putchar('\n');
			return EXIT_SUCCESS;
		}
	}

	scoped_heap_release(&maxheap);
	return EXIT_FAILURE;
}
