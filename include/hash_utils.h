#ifndef HASH_UTILS_H
#define HASH_UTILS_H

#include <stdint.h>
#include <stddef.h>
#include <math.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#include <float.h>

#include "number_utils.h"
#include "sort_utils.h"

#ifndef POINTER_SIZE
	#define POINTER_SIZE (sizeof(void*))
#endif

#define HASH_FNV_OFFSET_BASIS_64BIT (14695981039346656037ULL)
#define HASH_FNV_BIG_PRIME_64BIT (1099511628211ULL)

#define HASH_FNV_OFFSET_BASIS_32BIT (2166136261U)
#define HASH_FNV_BIG_PRIME_32BIT (16777619U)

#define HASH_FNV_OFFSET_BASIS ((POINTER_SIZE != 8u) ? (HASH_FNV_OFFSET_BASIS_32BIT) : (HASH_FNV_OFFSET_BASIS_64BIT))
#define HASH_FNV_BIG_PRIME ((POINTER_SIZE != 8u) ? (HASH_FNV_BIG_PRIME_32BIT) : (HASH_FNV_BIG_PRIME_64BIT))

// conjugado da razão áurea
#define HASH_MAGIC_NUMBER 0.6180339887

// max ssize_t range
#define HASH_SSIZE_MAX INT64_MAX

#define HASH_MAX_DIGIT_RANGE (10u)

typedef int64_t ssize_t;
typedef size_t(*hash_function_t)(ssize_t key, size_t table_size);
typedef double(*hash_deviation_function)(uint8_t ni, size_t n);
typedef size_t(*hash_prob_method_t)(ssize_t key, size_t k,
								  	size_t m, hash_function_t h);

typedef struct digit_deviation_pair_struct
{
	uint8_t digit;
	double deviation;
} digit_deviation_pair;

static bool digit_deviation_pair_comparison(const void* first,
									 		const void* second,
									 		size_t size)
{
	digit_deviation_pair* a = (digit_deviation_pair *) first;
	digit_deviation_pair* b = (digit_deviation_pair *) second;
	return a->deviation > b->deviation;
}

static inline size_t hash_by_division(ssize_t key, size_t m)
{
	// O AND bit-a-bit aqui é necessário para q possamos 'remover'
	// o bit de sinal da 'key'
	return (key & HASH_SSIZE_MAX) % m;
}

static inline size_t hash_by_fold(ssize_t key, size_t m)
{
	key &= HASH_SSIZE_MAX;

	size_t key_digit_cnt = get_digit_count(key, 10u);
    size_t group_size = get_digit_count(m, 10u);
    size_t group_sum = 0;

    static char key_str[32] = {0};
    static char group[32] = {0};

    size_t key_str_len = (sizeof(key_str) / sizeof(*key_str)) - 1u;
	size_t index = 0;

    snprintf(key_str, sizeof(key_str), "%zu", key);

    for (; index < key_str_len; index += group_size)
    {
        if (index + group_size <= key_str_len)
        {
            strncpy(group, key_str + index, group_size);
            group_sum += strtoull(group, NULL, 10);
        }
    }

    if (key_digit_cnt % group_size)
    {
        size_t offset = (index - group_size);
        size_t size = key_str_len - offset;
        strncpy(group, key_str + offset, size);
        group_sum += strtoull(group, NULL, 10);
    }

    return group_sum % m;
}

static inline size_t hash_by_mul(ssize_t key, size_t m)
{
	double value = (key & HASH_SSIZE_MAX) * HASH_MAGIC_NUMBER;
	value -= (ssize_t) value;
	return (size_t) ceil(m * value);
}

// https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
static size_t hash_fnv_util(const uint8_t* data, size_t len)
{
	size_t value = HASH_FNV_OFFSET_BASIS;
	const size_t p = HASH_FNV_BIG_PRIME;

	for (size_t index = 0; index < len; ++index)
	{
		value ^= (size_t)(data[index]);
		value *= p;
	}

    return value;
}

static inline size_t hash_by_fnv(ssize_t key, size_t m)
{
	key &= HASH_SSIZE_MAX;

	union { ssize_t v; uint8_t data[sizeof(key)]; } u = { key };
	size_t pre_hashed_key = hash_fnv_util(u.data, sizeof(key));

	return hash_by_division(pre_hashed_key, m);
}

static inline size_t hash_prob_method_linear(ssize_t key, size_t k,
								  	  	     size_t m, hash_function_t h)
{
	key &= HASH_SSIZE_MAX;
	return (h(key, m) + k) % m;
}

static inline size_t hash_prob_method_quadratic(ssize_t key, size_t k,
								  	  	 		size_t m, hash_function_t h)
{
	key &= HASH_SSIZE_MAX;

	const double c1 = 0.5;
	const double c2 = 0.5;

	const size_t x1 = (size_t)(c1 * k);
	const size_t x2 = (size_t)(c2 * (k * k));

	return (h(key, m) + x1 + x2) % m;
}

static inline size_t hash_prob_method_double_hashing(ssize_t key, size_t k,
													 size_t m, hash_function_t h1)
{
	hash_function_t h2 = hash_by_fnv;
	if (h1 == h2)
		h2 = hash_by_division;

	key &= HASH_SSIZE_MAX;

	size_t x1 = h1(key, m);
	size_t x2 = h2(key, m);

	// Tetrahedral number
	size_t x3 = ((k * k * k) - k) / 6;

	// We need x2 to be an odd number to ensure permutation throughout the table
	if (!x2 || !(x2 & 1))
		++x2;

	return (x1 + k * x2 + x3) % m;
}

static inline
digit_deviation_pair* prehash_by_digit_analisys(size_t ndigits, uint32_t* keyset,
												size_t keyset_size, hash_deviation_function dev_func)
{
	uint8_t digits[HASH_MAX_DIGIT_RANGE][HASH_MAX_DIGIT_RANGE] = { 0 };
	char digit_str[sizeof(uint32_t) * CHAR_BIT] = { 0 };
	digit_deviation_pair ddp[HASH_MAX_DIGIT_RANGE] = { 0 };

	uint32_t* beg = keyset;
	uint32_t* end = beg + keyset_size;

	for (; beg != end; ++beg)
	{
		uint32_t number = *beg;

		size_t number_digit_count = get_digit_count(number, 10u);
		snprintf(digit_str, sizeof(digit_str), "%hhu", number);

		for (size_t digit_index = 0; digit_index != number_digit_count; ++digit_index)
		{
			uint8_t kdigit = digit_str[digit_index] - '0';
			++digits[digit_index][kdigit - 1];
		}
	}

	for (uint8_t i = 0u; i < HASH_MAX_DIGIT_RANGE; ++i)
	{
		double accum = 0.0;

		for (uint8_t j = 0u; j < HASH_MAX_DIGIT_RANGE; ++j)
			if (digits[i][j])
				accum += dev_func(digits[i][j], keyset_size);

		ddp[i].deviation = accum ? accum : DBL_MAX;
		ddp[i].digit = i;
	}

	// sort deviation table in increasing order of deviation
	heap_sort(ddp, ddp + HASH_MAX_DIGIT_RANGE, sizeof(digit_deviation_pair), digit_deviation_pair_comparison);

	return memdup(ddp, sizeof(digit_deviation_pair) * HASH_MAX_DIGIT_RANGE);
}

static inline size_t hash_by_digit_analysis(ssize_t key, size_t m, uint8_t ndigits,
											digit_deviation_pair* dev_table)
{
	static char digit_str[sizeof(uint32_t) * CHAR_BIT] = {0};

	key &= HASH_SSIZE_MAX;
	size_t hashed_key = 0;

	snprintf(digit_str, sizeof(digit_str), "%zu", key);
	digit_str[ndigits] = '\0';

	for (uint8_t k = 0; k < ndigits; ++k)
	{
		hashed_key *= 10;
		hashed_key += (digit_str[dev_table[k].digit] - '0');
	}

	return hashed_key % m;
}

#endif
