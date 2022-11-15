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
    uint16 i; /* Initialization loop iterator */

    // Creating new page directory without initializing shared page tables
    for(i = 0; i < 5; i++) {
        if(identity_pt[i] == 0) {
            return (pd_t*) SYSERR;
        }
    }

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

    // Initialize page directory
    pdptr = (pd_t*) ((FRAME0 + frame_idx) * NBPG);
    memset(pdptr, 0, NBPG);

    // Add shared page tables
    {
        // Regions A - E2
        for(i = 0; i < 4; i++) {
            pdptr[i].pd_pres = 1;
            pdptr[i].pd_base = ((unsigned int) identity_pt[i]) / NBPG;
        }

        // Add shared page tables: Region G
        pdptr[REGION_G_PD].pd_pres = 1;
        pdptr[REGION_G_PD].pd_base = ((unsigned int) identity_pt[4]) / NBPG;
    }

    // Return initialized page
    return pdptr;
}
