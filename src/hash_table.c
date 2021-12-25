#include <assert.h>

#include "../include/utils.h"
#include "../include/hash_table.h"

bool hash_table_realloc(hash_table** pphtable, size_t value_size,
						size_t* ncollisions_ptr, double factor)
{
	if (!pphtable || !*pphtable || !factor)
		return false;

	hash_table* htable_ptr = *pphtable;
	hash_entry* table   = htable_ptr->data;

	size_t old_capacity = htable_ptr->capacity;
	size_t new_capacity = (size_t) ceil(old_capacity * factor);

	uint8_t prob_identity = get_prob_method_identity(htable_ptr->hash_prob_method);
	if (prob_identity == UINT8_MAX)
		return false;

	hash_table* new_hash_table = hash_table_create(new_capacity,
												   htable_ptr->hash_fptr,
												   prob_identity);
	if (!new_hash_table)
		return false;

	for (size_t index = 0; index < old_capacity; ++index)
	{
		if (table[index].status == HASH_ENTRY_STATUS_OCCUPIED)
		{
			bool success = hash_table_insert(table[index].key, table[index].value,
							  				 value_size, &new_hash_table, ncollisions_ptr);
			if (!success)
			{
				hash_table_release(&new_hash_table);
				return false;
			}
		}
	}

	hash_table_release(pphtable);
	*pphtable = new_hash_table;

	return true;
}

hash_table* hash_table_create(size_t nelems, hash_function_t hash_fn, uint8_t prob_method)
{
	if (!hash_fn)
		return NULL;

	hash_prob_method_t prob_method_fn = choose_prob_method(prob_method);
	if (!prob_method_fn)
		return NULL;

	size_t capacity = nelems ? round_up_to_power_of_2(nelems)
							 : HASH_TABLE_INITIAL_CAPACITY;

	size_t mem_size = capacity * sizeof(hash_entry);
	hash_entry* mem = (hash_entry *) malloc(mem_size);
	if (!mem)
		return NULL;

	for (hash_entry* mem_iter = mem; mem_iter != (mem + capacity); ++mem_iter)
		*mem_iter = (hash_entry){ .status = HASH_ENTRY_STATUS_FREE };

	hash_table* table = memdup(&(hash_table) {
		.hash_fptr = hash_fn,
		.capacity = capacity,
		.hash_prob_method = prob_method_fn,
		.data = mem
	}, sizeof(hash_table));

	if (table)
		return table;

	free(mem);
	free(table);
	return NULL;
}

bool hash_table_insert(ssize_t key, const void* value, size_t value_size,
					   hash_table** pphtable, size_t* number_of_collisions_ptr)
{
	hash_table* htable_ptr = *pphtable;

	if (!value || !pphtable || !htable_ptr)
		return false;

	if (hash_table_load_factor(htable_ptr) > HASH_TABLE_MAX_LOAD_FACTOR)
	{
		if (!hash_table_realloc(pphtable, value_size,
								number_of_collisions_ptr,
								HASH_TABLE_CAPACITY_FACTOR))
		{
			return false;
		}
	}

	// adjust htable_ptr point to new table
	htable_ptr = *pphtable;

	hash_entry* table = htable_ptr->data;
	size_t hash_index = htable_ptr->hash_fptr(key, htable_ptr->capacity);
	size_t nprobs = 0u;

	while (table[hash_index].status == HASH_ENTRY_STATUS_OCCUPIED &&
		   table[hash_index].key != key)
	{
		hash_index = htable_ptr->hash_prob_method(key, ++nprobs,
												  htable_ptr->capacity,
												  htable_ptr->hash_fptr);
	}

	if (number_of_collisions_ptr)
		*number_of_collisions_ptr = nprobs;

	if (table[hash_index].status == HASH_ENTRY_STATUS_DELETED ||
		table[hash_index].key == key)
	{
		free(table[hash_index].value);
	}
	else
		++htable_ptr->size;

	table[hash_index].key = key;
	table[hash_index].status = HASH_ENTRY_STATUS_OCCUPIED;
	table[hash_index].value = memdup(value, value_size);

	return true;
}

hash_entry* hash_table_search(ssize_t key, hash_table* htable_ptr)
{
	if (!htable_ptr)
		return NULL;

	size_t hash_index = htable_ptr->hash_fptr(key, htable_ptr->capacity);
	hash_entry* table = htable_ptr->data;
	size_t nprobs = 0;

	while (table[hash_index].status != HASH_ENTRY_STATUS_FREE)
	{
		if (table[hash_index].key == key &&
		    table[hash_index].status != HASH_ENTRY_STATUS_DELETED)
		{
			return &table[hash_index];
		}

		hash_index = htable_ptr->hash_prob_method(key, ++nprobs,
												  htable_ptr->capacity,
												  htable_ptr->hash_fptr);
	}

	return &table[hash_index];
}

void* hash_table_get(ssize_t key, hash_table* htable_ptr)
{
	hash_entry* bucket = hash_table_search(key, htable_ptr);
	if (!bucket)
		return NULL;

	if (bucket->status == HASH_ENTRY_STATUS_OCCUPIED)
		return bucket->value;

	return NULL;
}

void hash_table_release(hash_table** pphtable)
{
	if (!pphtable || !*pphtable)
		return;

	hash_table* htable = *pphtable;
	hash_entry* end = htable->data + htable->capacity;

	for (hash_entry* iter = htable->data; iter != end; ++iter)
	{
		if (iter->status == HASH_ENTRY_STATUS_OCCUPIED)
			free(iter->value);
	}

	free(htable->data);
	free(htable);
	*pphtable = NULL;
}

bool hash_table_remove(ssize_t key, hash_table* htable_ptr)
{
	hash_entry* bucket = hash_table_search(key, htable_ptr);
	if (!bucket)
		return false;

	if (bucket->status != HASH_ENTRY_STATUS_OCCUPIED)
		return false;

	free(bucket->value);
	bucket->value = NULL;
	bucket->status = HASH_ENTRY_STATUS_DELETED;

	--htable_ptr->size;
	return true;
}

double hash_table_load_factor(hash_table* htable_ptr)
{
	return htable_ptr->size / (double) htable_ptr->capacity;
}

bool hash_table_shrink(hash_table** pphtable, size_t value_size,
					   size_t* ncollisions_ptr, double min_load_factor)
{
	double load_factor = hash_table_load_factor(*pphtable);

	if (load_factor <= min_load_factor)
	{
		bool success = hash_table_realloc(pphtable, value_size,
										  ncollisions_ptr, min_load_factor);

		return success;
	}

	return false;
}
