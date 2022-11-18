/* getfreeframe.c - getfreeframe */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  getfreeframe  -  Return frame in region,
 *                   returning relative frame number with resp to invpt
 *------------------------------------------------------------------------
 */
fidx16 getfreeframe(region r) {
    uint16 idx; /* Iterator */
    uint16 lim; /* Limiting index */

    // Set starting iterator and limit
    switch(r) {
        case REGION_D: // [0, 1000) + 1024 = [1024, 2024)
            idx = FRAME0_D  - FRAME0;
            lim = FRAME0_E1 - FRAME0;
            break;

        case REGION_E1: // [1000, 2024) + 1024 = [2024, 3048)
            idx = FRAME0_E1 - FRAME0;
            lim = FRAME0_E2 - FRAME0;
            break;

        case REGION_E2: // [2024, 3072) + 1024 = [3038, 4096)
            idx = FRAME0_E2 - FRAME0;
            lim = FRAME0_F  - FRAME0;
            break;
    }

    // Find first free frame in this region
    for( ; idx < lim ; idx++) {
        if(invpt[idx].fr_state == FR_FREE) {
            return idx;
        }
    }

    // No free frame in this region, error
    return SYSERR;
}
