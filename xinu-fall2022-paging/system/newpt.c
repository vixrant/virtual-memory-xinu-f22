/* newpt.c - newpt */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  newpt  -  Create a new Page Table in Region D, returning pointer
 *------------------------------------------------------------------------
 */
pt_t *newpt(pid32 pid) {
    fidx16 frame_idx; /* Index of frame in inverted page table */
    pt_t *ptptr; /* Memory location */

    // Get a free frame
    frame_idx = getfreeframe(REGION_D);
    log_init("newpt %d - getfreeframe returned %d \n", frame_idx, pid);
    if(frame_idx == SYSERR) {
        return (pt_t*) SYSERR;
    }

    // Occupy it
    if(allocaframe(frame_idx, pid) == SYSERR) {
        return (pt_t*) SYSERR;
    }

    // Initialize page table
    ptptr = (pt_t*) ((FRAME0 + frame_idx) * NBPG);
    memset(ptptr, 0, NBPG);

    // Return initialized page
    return ptptr;
}
