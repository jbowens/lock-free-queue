#pragma once

/**
 * Compare and set macro.
 */
#define compare_and_set(loc, old_value, new_value) __sync_bool_compare_and_swap((void **)loc, old_value, new_value)
