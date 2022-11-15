/* getfreeframe.c - getfreeframe */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  getfreeframe  -  Return frame in region,
 *                   returning relative frame number with resp to invpt
 *------------------------------------------------------------------------
 */
int16 getfreeframe(region r) {
    uint16 idx; /* Iterator */
    uint16 lim; /* Limiting index */

    // Set starting iterator and limit
    switch(r) {
        case REGION_D: // [0, 1000) + 1024 = [1024, 2024)
            idx = 0;
            lim = NFRAMES_D;
            break;

        case REGION_E1: // [1000, 2024) + 1024 = [2024, 3048)
            idx = NFRAMES_D;
            lim = NFRAMES_D + NFRAMES_E1;
            break;

        case REGION_E2: // [2024, 3072) + 1024 = [3038, 4096)
            idx = NFRAMES_D + NFRAMES_E1;
            lim = NFRAMES_D + NFRAMES_E1 + NFRAMES_E2;
            break;
    }

    // Find first free frame in this region
    for( ; idx < lim ; idx++) {
        if(invpt[idx].fr_state == FR_FREE) {
            pdf("idx: %x \n", idx);
            return idx;
        }
    }

    // No free frame in this region, error
    return SYSERR;
}
