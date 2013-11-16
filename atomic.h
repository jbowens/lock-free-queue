#pragma once

/**
 * Compare and set. Intended only for 32 bit machines. Returns 1 if successful
 * 0 otherwise.
 */
static int compare_and_set(void *loc, void *old_value, void *new_value)
{
    void * ret = __sync_val_compare_and_swap((void **) loc, old_value, new_value);
    return ret == old_value ? 1 : 0;
}
