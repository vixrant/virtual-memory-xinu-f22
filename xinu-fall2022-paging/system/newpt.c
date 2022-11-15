/* newpt.c - newpt */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  newpt  -  Create a new Page Table in Region D, returning pointer
 *------------------------------------------------------------------------
 */
pt_t *newpt(void) {
    int16 frame_idx; /* Index of frame in inverted page table */
    frame_t *frptr; /* Pointer to frame in inverted page table */
    pt_t *ptptr; /* Memory location */

    // Get a free frame
    pdf("newpt - Finding frame \n");
    frame_idx = getfreeframe(REGION_D);
    if(frame_idx == SYSERR) {
        return (pt_t*) SYSERR;
    }

    // Occupy it
    frptr = &invpt[frame_idx];
    frptr->fr_state = FR_USEDT;
    frptr->fr_pid = currpid;

    // Initialize page table
    ptptr = (pt_t*) ((FRAME0 + frame_idx) * NBPG);
    memset(ptptr, 0, NBPG);

    // Return initialized page
    return ptptr;
}
