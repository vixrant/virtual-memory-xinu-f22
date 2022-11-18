/* newpd.c - newpd */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  newpd  -  Create a new Page Directory in Region D, returning pointer
 *------------------------------------------------------------------------
 */
pd_t *newpd(pid32 pid) {
    fidx16 frame_num; /* Index of frame in inverted page table */
    pd_t *pdptr; /* Memory location */
    uint16 i; /* Initialization loop iterator */

    // Creating new page directory
    // without initializing shared page tables
    // ss a system error
    for(i = 0; i < 5; i++) {
        if(identity_pt[i] == 0) {
            return (pd_t*) SYSERR;
        }
    }

    // Get a free frame
    frame_num = getfreeframe(REGION_D);
    pdfpg("newpd %d - getfreeframe returned %d \n", frame_num, pid);
    if(frame_num == SYSERR) {
        return (pd_t*) SYSERR;
    }

    // Occupy it
    if(allocaframe(frame_num, pid) == SYSERR) {
        return (pd_t*) SYSERR;
    }

    // Initialize page directory
    pdptr = (pd_t*) ((FRAME0 + frame_num) * NBPG);
    memset(pdptr, 0, NBPG);

    // Add shared page tables
    {
        // Regions A - E2
        for(i = 0; i < 4; i++) {
            pdptr[i].pd_pres = 1;
            pdptr[i].pd_base = ((uint32) identity_pt[i]) / NBPG;
        }

        // Add shared page tables: Region G
        pdptr[REGION_G_PD].pd_pres = 1;
        pdptr[REGION_G_PD].pd_base = ((uint32) identity_pt[4]) / NBPG;
    }

    // Return initialized page
    return pdptr;
}
