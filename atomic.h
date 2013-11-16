#pragma once

/**
 * A raw 32-bit cmpxchg adapted from linux. Takes a lock prefix for
 * optionally memfencing before.
 */
/*
#define __raw_cmpxchg(ptr, old_value, new_value)                        \
({                                                                      \
        void *__ret;                                                    \
        void *__old = (old_value);                                      \
        void *__new = (new_value);                                      \
        volatile void **__ptr = (volatile void **)(ptr);                \
        __asm__ volatile("cmpxchgl %2,%1"                               \
                : "=a" (__ret), "+m" (*__ptr)                           \
                : "r" (__new), "0" (__old)                              \
                : "memory");                                            \
        __ret;                                                          \
})*/

/**
 * Compare and set. Intended only for 32 bit machines. Returns 1 if successful
 * 0 otherwise.
 */
static int compare_and_set(void *loc, void *old_value, void *new_value)
{
    void * ret = __sync_val_compare_and_swap((void **) loc, old_value, new_value);
    return ret == old_value ? 1 : 0;
}
