#pragma once

#include <pthread.h>
#include "hazard_ptr.h"

/**
 * A daemon for reaping memory freed from lock-free data structures.
 *
 * @author jbowens, ejcaruso
 */

/**
 * A memory location ready to be freed by the reaper.
 */
typedef struct lockfree_freenode {
    struct lockfree_freenode * volatile lffn_next;
} lockfree_freenode_t;

/**
 * Parameters to the reapd daemon passed in when the reapd is initialized.
 */
typedef struct lockfree_reapd_attr {
    /* The hazard table to scan for pointers. */
    hazard_table_t *lfra_hazard_table;
    /* The function to call to actually reclaim memory. */
    void (*lfra_free_func)(void *);
    /* A pointer to the sentinel head of the free list used
     * by the lock-free data structure. */
    lockfree_freenode_t *lfra_free_list;
} lockfree_reapd_attr_t;

/**
 * Just for while we're testing in userland.
 */
typedef pthread_t lf_reaper_t;

/**
 * Spawns a new reaper.
 */
void lockfree_reapd_spawn(lockfree_reapd_attr_t *attr);

/**
 * Cleans up at the end.
 */
void lockfree_reapd_cleanup();

