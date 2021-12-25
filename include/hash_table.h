#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#define HASH_TABLE_API

#include <stdint.h>
#include <stdbool.h>
#include "hash_utils.h"

#define HASH_TABLE_INITIAL_CAPACITY (32)
#define HASH_TABLE_MAX_LOAD_FACTOR  (0.5)
#define HASH_TABLE_MIN_LOAD_FACTOR  (0.125)
#define HASH_TABLE_CAPACITY_FACTOR  (0x2)

#define HASH_PROBING_METHOD_LINEAR 	  		(0x00000001)
#define HASH_PROBING_METHOD_QUADRATIC 		(0x00000002)
#define HASH_PROBING_METHOD_DOUBLE_HASHING  (0x00000003)

#define HASH_ENTRY_STATUS_FREE 	   (0x0000007F)
#define HASH_ENTRY_STATUS_OCCUPIED (0x00000080)
#define HASH_ENTRY_STATUS_DELETED  (0x00000081)

typedef struct hash_entry_struct
{
	ssize_t key;
	void* value;
	uint8_t status;
} hash_entry;

typedef struct hash_table_struct
{
	hash_entry* data;
	size_t size;
	size_t capacity;
	hash_function_t hash_fptr;
	hash_prob_method_t hash_prob_method;
} hash_table;

static inline hash_prob_method_t choose_prob_method(uint8_t prob_method)
{
	switch (prob_method)
	{
		case HASH_PROBING_METHOD_LINEAR:
			return hash_prob_method_linear;
		case HASH_PROBING_METHOD_QUADRATIC:
			return hash_prob_method_quadratic;
		case HASH_PROBING_METHOD_DOUBLE_HASHING:
			return hash_prob_method_double_hashing;
		default:
			return NULL;
	}
}

static inline uint8_t get_prob_method_identity(hash_prob_method_t prob_method)
{
	hash_prob_method_t prob_methods[] =
	{
		hash_prob_method_linear,
		hash_prob_method_quadratic,
		hash_prob_method_double_hashing
	};

	uint8_t n = (uint8_t)(sizeof(prob_methods) / sizeof(prob_methods[0]));

	for (uint8_t i = 0u; i < n; ++i)
	{
		if (prob_method == prob_methods[i])
		{
			return (i + 1);
		}
	}

	return UINT8_MAX;
}

bool HASH_TABLE_API hash_table_realloc(hash_table** pphtable, size_t value_size,
									   size_t* ncollisions_ptr, double factor);

hash_table* HASH_TABLE_API hash_table_create(size_t nelems,
											 hash_function_t hash_fn,
											 uint8_t prob_method);

bool HASH_TABLE_API hash_table_insert(ssize_t key,
									  const void* value,
									  size_t value_size,
					   				  hash_table** htable_ptr,
									  size_t* number_of_collisions_ptr);

hash_entry* HASH_TABLE_API hash_table_search(ssize_t key, hash_table* htable_ptr);

void* HASH_TABLE_API hash_table_get(ssize_t key, hash_table* htable_ptr);

bool HASH_TABLE_API hash_table_remove(ssize_t key, hash_table* htable_ptr);

void HASH_TABLE_API hash_table_release(hash_table** pphtable);

double HASH_TABLE_API hash_table_load_factor(hash_table* htable_ptr);

bool HASH_TABLE_API hash_table_shrink(hash_table** pphtable, size_t value_size,
					  				  size_t* ncollisions_ptr, double min_load_factor);

#endif
