#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "../include/mem_utils.h"
#include "../include/hash_utils.h"
#include "../include/hash_table.h"

static void print_info(hash_table* hashmap)
{
	printf("size = %zu\ncapacity = %zu\n", hashmap->size, hashmap->capacity);
}

int main(int argc, char** argv)
{
	hash_table* hashmap = hash_table_create(128, hash_by_fold,
											HASH_PROBING_METHOD_QUADRATIC);

	if (hashmap)
	{
		print_info(hashmap);
		size_t ncollisions = 0;

		if (hash_table_insert(-512, &(int){7}, sizeof(int),  &hashmap, &ncollisions)  &&
			hash_table_insert(128,  &(int){8},  sizeof(int), &hashmap, &ncollisions)  &&
			hash_table_insert(256,  &(int){12}, sizeof(int), &hashmap, &ncollisions)  &&
			hash_table_insert(91,   &(int){80}, sizeof(int), &hashmap, &ncollisions)  &&
			hash_table_insert(22,   &(int){45}, sizeof(int), &hashmap, &ncollisions)  &&
			hash_table_insert(1000, &(int){55}, sizeof(int), &hashmap, &ncollisions))
		{
			// hash_table_shrink(&hashmap, sizeof(int), NULL, HASH_TABLE_MIN_LOAD_FACTOR);
			print_info(hashmap);

			for (size_t i = 0; i < hashmap->capacity; ++i)
			{
				if (hashmap->data[i].status == HASH_ENTRY_STATUS_OCCUPIED)
				{
					printf("kvp (%zu): %d => %d\n", i, (int) hashmap->data[i].key,
												   *(int *) hashmap->data[i].value);
				}
			}

			printf("[+] Number of collisions: %zu\n", ncollisions);
		}

		printf("%d\n", *(int *) hash_table_get(-512, hashmap));
		printf("load factor = %.2f\n", hash_table_load_factor(hashmap));
		hash_table_release(&hashmap);
	}

	return EXIT_FAILURE;
}
