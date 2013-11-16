#pragma once

#include <stdint.h>

typedef struct stamped_ref {
    void *ptr;
    uint32_t stamp;
} stamped_ref;

/**
 * Compare and set macro.
 */
#define compare_and_set(loc, old_value, new_value) __sync_bool_compare_and_swap((void **)loc, old_value, new_value)
