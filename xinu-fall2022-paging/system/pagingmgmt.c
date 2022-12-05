/* pagingmgmt.c - newpd, newpt, deletept */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  deletept -  Deletes a page directory entry and frees its frame
 *------------------------------------------------------------------------
 */
void deletept(pd_t *pde) {
    log_mem("deletept - deleting PT at frame %d \n", pt_frame_num);
    
    // Get frame of page table
    fidx16 frame_num = pde->pd_base;

    // Free frame
    invfreeframe(frame_num);

    // Mark page table as not present
    pde->pd_pres = 0;
}

/*------------------------------------------------------------------------
 *  newpt  -  Create a new Page Table in Region D, returning pointer
 *------------------------------------------------------------------------
 */
pt_t *newpt(pid32 pid) {
    fidx16 frame_num; /* Index of frame in inverted page table */
    pt_t *ptptr; /* Memory location */

    log_init("newpt %d - setting up page table \n", pid);

    // Get a free frame
    frame_num = getfreeframe(REGION_D);
    if(frame_num == SYSERR) {
        return (pt_t*) SYSERR;
    }

    // Occupy it
    if(invtakeframe(frame_num, pid, FR_PTEUNUSED) == SYSERR) {
        return (pt_t*) SYSERR;
    }

    // Initialize page table
    ptptr = (pt_t*) (frame_num * NBPG);
    memset(ptptr, 0, NBPG);

    // Return initialized page
    return ptptr;
}

/*------------------------------------------------------------------------
 *  newpd  -  Create a new Page Directory in Region D, returning pointer
 *------------------------------------------------------------------------
 */
pd_t *newpd(pid32 pid) {
    fidx16 frame_num; /* Index of frame in inverted page table */
    pd_t *pdptr; /* Memory location */

    log_init("newpt %d - setting up page table \n", pid);

    // Get a free frame
    frame_num = getfreeframe(REGION_D);
    if(frame_num == SYSERR) {
        return (pd_t*) SYSERR;
    }

    // Occupy it
    if(invtakeframe(frame_num, pid, FR_PTEUNUSED) == SYSERR) {
        return (pd_t*) SYSERR;
    }

    // Initialize page directory
    pdptr = (pd_t*) (frame_num * NBPG);
    memset(pdptr, 0, NBPG);

    // Add shared page tables
    {
        uint16 i; /* Initialization loop iterator */

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
