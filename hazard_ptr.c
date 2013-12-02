#include "hazard_ptr.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static hazard_table_t *hazard_table_alloc()
{
    /* TODO: Slab allocate dat bitch. */
    hazard_table_t *table = (hazard_table_t *) malloc(sizeof(hazard_table_t));
    memset(table, 0, sizeof(hazard_table_t));
    return table;
}

static void hazard_table_free(hazard_table_t *table)
{
    /* TODO: Slab allocator free dat bitch. */
    free(table);
}

int hazard_table_search(hazard_table_t *table, void *ptr)
{
    hazard_table_t *hazard;

    __sync_synchronize();

    int i, j;
    for (hazard = table; hazard != 0; hazard = hazard->ht_next_table) {
        for (i = 0; i < HAZARD_TABLE_SIZE; ++i) {
            for (j = 0; j < HAZARD_ENTRY_SIZE; ++j) {
                if (hazard->ht_entries[i].he_ptrs[j] == ptr) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

void hazard_ptr_init(hazard_table_t *table)
{
    memset(table, 0, sizeof(hazard_table_t));
}

hazard_entry_t *hazard_ptr_getentry(hazard_table_t *table, uint32_t tid)
{
    hazard_table_t *hazard;;

    uint32_t off = tid;
    for (hazard = table; hazard->ht_next_table != 0; hazard = hazard->ht_next_table) {
        if (off < HAZARD_TABLE_SIZE) {
            return &hazard->ht_entries[off];
        }
        off -= HAZARD_TABLE_SIZE;
    }

    /* We need to extend to the hazard table. */
    hazard_table_t *new_table = 0;

    while (off >= HAZARD_TABLE_SIZE) {
        if (new_table == 0) {
            new_table = hazard_table_alloc();
        }

        /* Try and extend the current table with our new table. */
        if (compare_and_set(&hazard->ht_next_table, 0, new_table)) {
            /* We threaded our new table on. Our new table is now in use */
            new_table = 0;
        }
        hazard = hazard->ht_next_table;
        off -= HAZARD_TABLE_SIZE;
    }

    if (new_table) {
        /* We have an unused table from when someone beat us to the punch
         * extending the hazard table. */
        hazard_table_free(new_table);
    }

    return &hazard->ht_entries[off];
}

int hazard_ptr_add(hazard_entry_t *entry, void *ptr)
{
    int i;
    for (i = 0; i < HAZARD_ENTRY_SIZE; ++i) {
        if (entry->he_ptrs[i] == 0) {
            entry->he_ptrs[i] = ptr;
            return 1;
        }
    }
    return 0;
}

int hazard_ptr_remove(hazard_entry_t *entry, void *ptr)
{
    int i;
    for (i = 0; i < HAZARD_TABLE_SIZE; ++i) {
        if (entry->he_ptrs[i] == ptr) {
            entry->he_ptrs[i] = 0;
            return 1;
        }
    }
    return 0;
}

