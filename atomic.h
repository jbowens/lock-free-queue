#pragma once

/**
 * Compare and set. Intended only for 32 bit machines. Returns 1 if successful
 * 0 otherwise.
 */
#define compare_and_set(loc, old_value, new_value) __sync_bool_compare_and_swap((void **)loc, old_value, new_value)
