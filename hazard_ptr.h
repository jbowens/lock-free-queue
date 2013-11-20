#pragma once

#include "atomic.h"

/**
 * A hazard pointer library for use in lock-free data structures.
 *
 * @author jbowens, ejcaruso
 */

#define HAZARD_ENTRY_SIZE 4
#define HAZARD_TABLE_SIZE 8

/*
 * Table of hazard pointers.
 */
typedef struct hazard_entry {
    void *he_ptrs[HAZARD_ENTRY_SIZE];
} hazard_entry_t;

typedef struct hazard_table {
    hazard_entry_t ht_entries[HAZARD_TABLE_SIZE];
    struct hazard_table *ht_next_table;
} hazard_table_t;

/**
 * Initializes a harzard table to be empty.
 */
void hazard_ptr_init(hazard_table_t *table);

/**
 * Retrieves a pointer to a harzard entry in the given harzard table, corresponding
 * to the given thread id. If there is no entry yet for the given thread id, the
 * table will be expanded in order to include entries for the given tid.
 *
 * @param table the table to lookup a tid for
 * @param tid the thread id to look up (should be 0 to n)
 */
hazard_entry_t *hazard_ptr_getentry(hazard_table_t *table, uint32_t tid);

/**
 * Adds the given pointer to the given hazard table entry. Returns 1 on
 * success, 0 if the given entry is full.
 *
 * @param entry the hazard table entry to add the pointer to
 * @param ptr the pointer to add a hazard pointer for
 */
int hazard_ptr_add(hazard_entry_t *entry, void *ptr);

/**
 * Removes the given pointer from the given hazard table entry. Returns
 * 1 on success or 0 if the pointer doesn't exist within the given
 * entry.
 */
int hazard_ptr_remove(hazard_entry_t *entry, void *ptr);

