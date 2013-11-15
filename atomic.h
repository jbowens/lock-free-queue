#pragma once

typedef struct {
        int counter;
} atomic_t;

/* Static initializer */
#define ATOMIC_INIT(i)  { (i) }

#define atomic_read(v)  (*(volatile int *)&(v)->counter)

#define atomic_set(v, i) (((v)->counter) = (i))

/**
 * A raw 32-bit cmpxchg stolen from linux. Takes a lock prefix for
 * optionally memfencing before.
 */
#define __raw_cmpxchg(ptr, old_value, new_value, lock)                  \
({                                                                      \
        __typeof__(*(ptr)) __ret;                                       \
        __typeof__(*(ptr)) __old = (old_value);                         \
        __typeof__(*(ptr)) __new = (new_value);                         \
        volatile uint32_t *__ptr = (volatile uint32_t *)(ptr);          \
        __asm__ volatile(lock "cmpxchgl %2,%1"                          \
                : "=a" (__ret), "+m" (*__ptr)                           \
                : "r" (__new), "0" (__old)                              \
                : "memory");                                            \
        __ret;                                                          \
})

/**
 * Compare and exchange on atomic_t's
 */
#define atomic_cmpxchg(v, old_val, new_val)                                 \
        __raw_cmpxchg(&v->counter, old_val, new_val, "")

/**
 * Compare and set on an atomic_t. Returns 1 if successful instead of
 * the old value.
 */
static int atomic_cas(atomic_t *v, int old_value, int new_value)
{
        int ret = atomic_cmpxchg(v, old_value, new_value);
        return ret == old_value ? 1 : 0;
}
