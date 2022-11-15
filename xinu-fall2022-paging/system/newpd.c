/* newpd.c - newpd */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  newpd  -  Create a new Page Directory in Region D, returning pointer
 *------------------------------------------------------------------------
 */
pd_t *newpd(void) {
    int16 frame_idx; /* Index of frame in inverted page table */
    frame_t *frptr; /* Pointer to frame in inverted page table */
    pd_t *pdptr; /* Memory location */
    uint16 fi; /* Initialization loop iterator */

    // Get a free frame
    pdf("newpd - Finding frame \n");
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
    for (fi = 0; fi < NENTRIES; fi++) {
        pdptr[fi].pd_pres   = 0; // Page present by default
        pdptr[fi].pd_write  = 0; // Writable
        pdptr[fi].pd_user   = 0; // No protection in this lab
        pdptr[fi].pd_pwt    = 0; // Don't write through
        pdptr[fi].pd_pcd    = 0; // Cache definitely
        pdptr[fi].pd_acc    = 0; // Not accessed currently
        pdptr[fi].pd_mbz    = 0; // Must not be zero
        pdptr[fi].pd_fmb    = 0; // Pages are 4KB
        pdptr[fi].pd_global = 0; // What is this?
        pdptr[fi].pd_avail  = 0; // For custom use
        pdptr[fi].pd_base   = 0; // Dummy
    }

    // Return initialized page
    return pdptr;
}
