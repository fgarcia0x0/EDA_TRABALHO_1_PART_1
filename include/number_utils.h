#ifndef NUMBER_UTILS_H
#define NUMBER_UTILS_H

#include <stdint.h>
#include <stddef.h>

static size_t get_digit_count(size_t value, size_t base)
{
    size_t num_digits = 0u;

    for (; value; value /= base)
         ++num_digits;

    return num_digits;
}

#endif
