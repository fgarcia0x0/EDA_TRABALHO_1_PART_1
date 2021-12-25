#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <errno.h>

#include "../include/utils.h"
#include "../include/scoped_heap.h"

typedef struct parsed_data_struct
{
	size_t vector_size;
	int* vector;
} parsed_data;

bool parse_args(int arg_cnt, char** argv, parsed_data* parsed_data_ptr);
void random_fill(int* begin, int* end, int m);
void sort_data(int* begin, int* end);

int main(int argc, char** argv)
{
	parsed_data data = {0};

	if (!parse_args(argc, argv, &data))
		return EXIT_FAILURE;

	random_fill(data.vector, data.vector + data.vector_size, 100000);
	sort_data(data.vector, data.vector + data.vector_size);
}

bool parse_args(int arg_cnt, char** argv, parsed_data* parsed_data_ptr)
{
	if (arg_cnt < 2 || !argv[1])
		return false;

	char* rest = NULL;

	size_t n = strtoull(argv[1], &rest, 10);
	if (n == SIZE_MAX || errno == ERANGE)
		return false;

	int* vector = (int *) malloc(n * sizeof(int));
	if (!vector)
		return false;

	*parsed_data_ptr = (parsed_data){ .vector = vector, .vector_size = n };
	return true;
}

void random_fill(int* begin, int* end, int m)
{
	srand((unsigned)time(NULL));
	for (int* iter = begin; iter != end; ++iter)
		*iter = rand() % m;
}

void sort_data(int* begin, int* end)
{
	size_t n = (size_t)(end - begin);
	int* tmp = begin;
	int* dup = (int *) memdup(begin, n * sizeof(int));

	time_t t1 = clock();
	heap_sort(begin, end, sizeof(int), greater_than_i32);
	time_t t2 = clock();

	free(tmp);
	double heap_time = (double)(t2 - t1) / CLOCKS_PER_SEC;
	printf("[+] Heap-Sort time for %zu elements: %.8f seconds\n", n, heap_time);

	t1 = clock();
	insertion_sort(dup, dup + n, sizeof(int), less_than_i32);
	t2 = clock();

	double isort_time = (double)(t2 - t1) / CLOCKS_PER_SEC;
	free(dup);

	printf("[+] Insertion-Sort time for %zu elements: %.8f seconds\n",n, isort_time);
	printf("[+] Finished\n");
}
