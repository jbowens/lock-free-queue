#pragma once

/**
 * Compare and set. Intended only for 32 bit machines. Returns 1 if successful
 * 0 otherwise.
 */
static int compare_and_set(void *loc, void *old_value, void *new_value)
{
    return __sync_bool_compare_and_swap((void **) loc, old_value, new_value);
}
