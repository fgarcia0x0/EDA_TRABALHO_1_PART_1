#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <errno.h>

#include "../include/mem_utils.h"
#include "../include/hash_utils.h"
#include "../thirdy-party/mtwister/mtwister.h"

#define HASH_TABLE_SIZE (100000u)
#define HASH_TABLE_SENTINEL (0xffu)

typedef struct parsed_data_struct
{
	size_t key_set_size;
} parsed_data;

typedef struct hash_table_concept_struct
{
	uint32_t* table_ptr;
	hash_function_t hash_fptr;
	size_t ncollisions;
} hash_table_concept;

bool parse_args(int arg_cnt, char** argv, parsed_data* parsed_data_ptr);
void random_fill(uint32_t* begin, uint32_t* end, uint32_t m);
bool measure(size_t n);

int main(int argc, char** argv)
{
	parsed_data data = {0};

	if (!parse_args(argc, argv, &data))
	{
		fprintf(stderr, "FAILURE !\n");
		return EXIT_FAILURE;
	}

	if (!measure(data.key_set_size))
	{
		fprintf(stderr, "FAILURE !\n");
		return EXIT_FAILURE;
	}

	printf("[+] Finished\n");
	return EXIT_SUCCESS;
}

static double hash_deviation_func_1(uint8_t ni, size_t n)
{
	return pow((double)ni - n / 10.0, 2);
}

static double hash_deviation_func_2(uint8_t ni, size_t n)
{
	return fabs((double)ni - n / 10.0);
}

bool parse_args(int arg_cnt, char** argv, parsed_data* parsed_data_ptr)
{
	if (arg_cnt < 2 || !argv[1])
		return false;

	size_t n = strtoull(argv[1], NULL, 10);
	if (n == SIZE_MAX || errno == ERANGE)
		return false;

	*parsed_data_ptr = (parsed_data){ .key_set_size = n };
	return true;
}

void random_fill(uint32_t* begin, uint32_t* end, uint32_t m)
{
	MTRand r = seedRand((unsigned) time(NULL));
	for (uint32_t* iter = begin; iter != end; ++iter)
		*iter = (uint32_t)genRandLong(&r) % m;
}

bool measure(size_t n)
{
	uint32_t* key_vector = create_vector(n, sizeof(uint32_t), false);
	if (!key_vector)
		return false;

	bool ret = false;
	random_fill(key_vector, key_vector + n, 2000000000U);

	// create deviation table for 2 deviation functions
	size_t mdigits = get_digit_count(n, 10u);
	digit_deviation_pair* dev_table_1 = prehash_by_digit_analisys(mdigits, key_vector, n, hash_deviation_func_1);
	digit_deviation_pair* dev_table_2 = prehash_by_digit_analisys(mdigits, key_vector, n, hash_deviation_func_2);

	if (!dev_table_1 || !dev_table_2)
		return false;

	uint32_t* table1 = create_vector(HASH_TABLE_SIZE, sizeof(uint32_t), true);
	uint32_t* table2 = create_vector(HASH_TABLE_SIZE, sizeof(uint32_t), true);
	uint32_t* table3 = create_vector(HASH_TABLE_SIZE, sizeof(uint32_t), true);
	uint32_t* table4 = create_vector(HASH_TABLE_SIZE, sizeof(uint32_t), true);
	uint32_t* table5 = create_vector(HASH_TABLE_SIZE, sizeof(uint32_t), true);

	hash_table_concept hash_tables[] =
	{
		(hash_table_concept){ table1, hash_by_division, 0u },
		(hash_table_concept){ table2, hash_by_fold, 0u },
		(hash_table_concept){ table3, hash_by_mul, 0u },
	};

	size_t hash_tables_size = 3u;
	size_t digit_analisys_func1_collisions = 0;
	size_t digit_analisys_func2_collisions = 0;

	if (table1 && table2 && table3 && table4 && table5)
	{
		for (size_t i = 0; i < n; ++i)
		{
			for (size_t table_index = 0; table_index < hash_tables_size; ++table_index)
			{
				hash_table_concept* target = &hash_tables[table_index];
				size_t hash_index = target->hash_fptr(key_vector[i], HASH_TABLE_SIZE);

				if (target->table_ptr[hash_index] == HASH_TABLE_SENTINEL)
					++(target->ncollisions);
				else
					target->table_ptr[hash_index] = HASH_TABLE_SENTINEL;
			}

			// Special Case
			size_t hash_index = hash_by_digit_analysis(key_vector[i], HASH_TABLE_SIZE, mdigits, dev_table_1);

			// hash for table deviation 1
			if (table4[hash_index] == HASH_TABLE_SENTINEL)
				++digit_analisys_func1_collisions;
			else
				table4[hash_index] = HASH_TABLE_SENTINEL;

			// hash for table deviation 2
			hash_index = hash_by_digit_analysis(key_vector[i], HASH_TABLE_SIZE, mdigits, dev_table_2);
			if (table5[hash_index] == HASH_TABLE_SENTINEL)
				++digit_analisys_func2_collisions;
			else
				table5[hash_index] = HASH_TABLE_SENTINEL;
		}

		printf("[+] division method = %zu collisions\n", hash_tables[0].ncollisions);
		printf("[+] fold method = %zu collisions\n", hash_tables[1].ncollisions);
		printf("[+] mul method = %zu collisions\n", hash_tables[2].ncollisions);

		printf("[+] digit analisys [f1] method = %zu collisions\n", digit_analisys_func1_collisions);
		printf("[+] digit analisys [f2] method = %zu collisions\n", digit_analisys_func2_collisions);

		ret = true;
	}

	free(dev_table_1);
	free(dev_table_2);
	free(table1);
	free(table2);
	free(table3);
	free(table4);
	free(table5);

	return ret;
}
