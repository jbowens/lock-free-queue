#pragma once

/**
 * A raw 32-bit cmpxchg adapted from linux. Takes a lock prefix for
 * optionally memfencing before.
 */
#define __raw_cmpxchg(ptr, old_value, new_value, lock)                  \
({                                                                      \
        void *__ret;                                                    \
        void *__old = (old_value);                                      \
        void *__new = (new_value);                                      \
        volatile void **__ptr = (volatile void **)(ptr);          \
        __asm__ volatile(lock "cmpxchgl %2,%1"                          \
                : "=a" (__ret), "+m" (*__ptr)                           \
                : "r" (__new), "0" (__old)                              \
                : "memory");                                            \
        __ret;                                                          \
})

/**
 * Compare and exchange on atomic_t's
 */
#define atomic_cmpxchg(ptr, old_val, new_val)                                 \
        __raw_cmpxchg(ptr, old_val, new_val, "")

/**
 * Compare and set. Intended only for 32 bit machines. Returns 1 if successful
 * 0 otherwise.
 */
static int compare_and_set(void *loc, void *old_value, void *new_value)
{
        void *ret = atomic_cmpxchg(loc, old_value, new_value);
        return ret == old_value ? 1 : 0;
}
