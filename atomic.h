#pragma once

#include <stdint.h>

typedef struct stamped_ref {
    void *ptr;
    union {
        uint32_t stamp;
        void *_align;
    };
} stamped_ref_t;

void stamped_ref_init(volatile stamped_ref_t *sr, void *p);

#define STAMPED_REF(p) { .ptr = (p), .stamp = 0 }

/**
 * Compare and set macro.
 */
#define compare_and_set(loc, old_value, new_value) __sync_bool_compare_and_swap((void **)loc, old_value, new_value)

uint32_t stamped_ref_cas(
    volatile stamped_ref_t *sr,
    void *old_ref,
    uint32_t old_stamp,
    void *new_ref,
    uint32_t new_stamp
);

void *stamped_ref_get(volatile stamped_ref_t *sr, uint32_t *stamp);
