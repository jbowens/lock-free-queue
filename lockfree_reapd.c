#include <stdlib.h>
#include <sched.h>
#include "lockfree_reapd.h"

lf_reaper_t *lockfree_reapd_spawn(lockfree_reapd_attr_t *attr)
{
    pthread_t *thread = malloc(sizeof(pthread_t));
    pthread_create(thread, 0, &lockfree_reapd_main, attr);
    return thread;
}

void *lockfree_reapd_main(void *arg)
{
    lockfree_reapd_attr_t *attr = (lockfree_reapd_attr_t *) arg;

    for (;;)
    {

        lockfree_freenode_t *prev = attr->lfra_free_list;
        for (lockfree_freenode_t *curr = prev->lffn_next; curr != 0; curr = curr->lffn_next)
        {
            /* Is the pointer to this item still in the hazard table? */
            if (!hazard_table_search(attr->lfra_hazard_table, curr)) {
                /* This pointer is no longer referenced. Free it. */
                attr->lfra_free_func(curr);
                /* Unlink the node from the free list. */
                prev->lffn_next = curr->lffn_next;
            } else {
                /* We didn't unlink curr, so move prev forward to curr. */
                prev = curr;
            }
        }

        /* TODO: Update this to actually go to sleep. */
        sched_yield();

    }

    return 0;
}
