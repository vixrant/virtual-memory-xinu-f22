/* newpt.c - newpt */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  newpt  -  Create a new Page Table in Region D, returning pointer
 *------------------------------------------------------------------------
 */
pt_t *newpt(pid32 pid) {
    fidx16 frame_num; /* Index of frame in inverted page table */
    pt_t *ptptr; /* Memory location */

    // Get a free frame
    frame_num = getfreeframe(REGION_D);
    pdfpg("newpt %d - getfreeframe returned %d \n", frame_num, pid);
    if(frame_num == SYSERR) {
        return (pt_t*) SYSERR;
    }

    // Occupy it
    if(allocaframe(frame_num, pid) == SYSERR) {
        return (pt_t*) SYSERR;
    }

    // Initialize page table
    ptptr = (pt_t*) ((FRAME0 + frame_num) * NBPG);
    memset(ptptr, 0, NBPG);

    // Return initialized page
    return ptptr;
}
