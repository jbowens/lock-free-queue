#include <stdlib.h>
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

    return 0;
}
