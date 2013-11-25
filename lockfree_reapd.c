#include <stdlib.h>
#include <sched.h>
#include "atomic.h"
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

        lockfree_freenode_t *sentinel_head = attr->lfra_free_list;

        while (sentinel_head->lffn_next == 0) {
            /* TODO: Add a sleep while the list is empty. */
            sched_yield();
        }

        lockfree_freenode_t *gobble_head = sentinel_head->lffn_next;
        /* Keep trying to steal the list until we're successful. */
        while (!compare_and_set(&sentinel_head->lffn_next, gobble_head, 0)) {
            gobble_head = sentinel_head->lffn_next;
        }

        /* gobble_head is now the head of a list of nodes that are available for
         * freeing. We just need to free them all at our own convenience. The list
         * will not be modified by any other thread, because we just stole it away.
         */
        while (gobble_head != 0) {
            lockfree_freenode_t *prev = gobble_head;
            for (lockfree_freenode_t *curr = gobble_head; curr != 0; curr = curr->lffn_next) {
                if (!hazard_table_search(attr->lfra_hazard_table, curr)) {
                    /* This node is no longer in the hazard table and it's safe to free it. */
                    if (curr == gobble_head) {
                        /* This is the head. Just move the head forward. */
                        gobble_head = curr->lffn_next;
                    } else {
                        /* This node is within the list. Unlink it. */
                        prev->lffn_next = curr->lffn_next;
                    }
                    attr->lfra_free_func(curr);
                } else {
                    prev = curr;
                }
            }
        }

        /* TODO: Update this to actually go to sleep. */
        sched_yield();

    }

    return 0;
}
