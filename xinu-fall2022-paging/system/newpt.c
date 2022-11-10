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
    uint16 fi; /* Initialization loop iterator */

    // Get a free frame
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

    // Treat it as an array of 1024 entries
    for (fi = 0; fi < NENTRIES; fi++) {
        ptptr[fi].pt_pres   = 0; // Page absent by default
        ptptr[fi].pt_write  = 1; // Writable
        ptptr[fi].pt_user   = 0; // No protection in this lab
        ptptr[fi].pt_pwt    = 0; // Don't write through
        ptptr[fi].pt_pcd    = 0; // Cache definitely
        ptptr[fi].pt_acc    = 0; // Not accessed currently
        ptptr[fi].pt_dirty  = 0; // Page not written
        ptptr[fi].pt_mbz    = 0; // Must not be zero
        ptptr[fi].pt_global = 0; // What is this?
        ptptr[fi].pt_avail  = 0; // For custom use
        ptptr[fi].pt_base   = -1; // Dummy
    }

    // Return initialized page
    return ptptr;
}
