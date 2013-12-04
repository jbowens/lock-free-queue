#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include "atomic.h"
#include "lockfree_reapd.h"

static pthread_t reapd;
static int reap = 1;

static void *lockfree_reapd_main(void *arg)
{
    lockfree_reapd_attr_t *attr = (lockfree_reapd_attr_t *) arg;
    lockfree_freenode_t *sentinel_head = attr->lfra_free_list;

    while (reap || sentinel_head->lffn_next != 0)
    {

        while (reap && sentinel_head->lffn_next == 0) {
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
            /* Search for something in the list that's ready to be freed.
             */
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
                    curr->lffn_next = 0;
                    attr->lfra_free_func(curr);
                    /* Do NOT continue traversing because curr->lffn_next is now garbage. Restart
                     * the traversal on the modified list. */
                    break;
                }
            }
        }

        sched_yield();

    }

    fprintf(stderr, "reapd quitting\n");
    fprintf(stderr, "reap: %d, free list head: %p\n", reap, sentinel_head->lffn_next);

    return 0;
}

void lockfree_reapd_spawn(lockfree_reapd_attr_t *attr)
{
    pthread_create(&reapd, 0, &lockfree_reapd_main, attr);
}

void lockfree_reapd_cleanup()
{
    reap = 0;
    pthread_join(reapd, 0);
}
