/* newpd.c - newpd */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  newpd  -  Create a new Page Directory in Region D, returning pointer
 *------------------------------------------------------------------------
 */
pd_t *newpd() {
    int16 frame_idx; /* Index of frame in inverted page table */
    frame_t *frptr; /* Pointer to frame in inverted page table */
    pd_t *pdptr; /* Memory location */
    uint16 i; /* Initialization loop iterator */

    // Get a free frame
    frame_idx = getfreeframe(REGION_D);
    if(frame_idx == SYSERR) {
        return (pd_t*) SYSERR;
    }

    // Occupy it
    frptr = &invpt[frame_idx];
    frptr->fr_state = FR_USEDD;
    frptr->fr_pid = currpid;

    // Initialize page table
    pdptr = (pd_t*) ((FRAME0 + frame_idx) * NBPG);

    // Treat it as an array of 1024 entries
    for (i = 0; i < NENTRIES; i++) {
        pdptr[i].pd_pres   = 0; // Page absent by default
        pdptr[i].pd_write  = 1; // Writable
        pdptr[i].pt_user   = 0; // No protection in this lab
        pdptr[i].pt_pwt    = 0; // Don't write through
        pdptr[i].pt_pcd    = 0; // Cache definitely
        pdptr[i].pt_acc    = 0; // Not accessed currently
        pdptr[i].pt_mbz    = 0; // Must not be zero
        pdptr[i].pd_fmb    = 0; // Pages are 4KB
        pdptr[i].pt_global = 0; // What is this?
        pdptr[i].pt_avail  = 0; // For custom use
        pdptr[i].pt_base   = -1; // Dummy
    }

    // Return initialized page
    return pdptr;
}
