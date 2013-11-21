#include <string.h>
#include "atomic.h"

void stamped_ref_init(volatile stamped_ref_t *sr, void *p) {
    memset((void *) sr, 0, sizeof(stamped_ref_t));
    sr->ptr = p;
}
