/* newpt.c - newpt */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  newpt  -  Create a new Page Table in Region D, returning pointer
 *------------------------------------------------------------------------
 */
pt_t *newpt(pid32 pid) {
    int16 frame_num; /* Index of frame in inverted page table */
    frame_t *frptr; /* Pointer to frame in inverted page table */
    pt_t *ptptr; /* Memory location */

    // Get a free frame
    frame_num = getfreeframe(REGION_D);
    pdfpg("newpt %d - getfreeframe returned %d \n", frame_num, pid);
    if(frame_num == SYSERR) {
        return (pt_t*) SYSERR;
    }

    // Occupy it
    frptr = &invpt[frame_num];
    frptr->fr_state = FR_USEDT;
    frptr->fr_pid = pid;

    // Initialize page table
    ptptr = (pt_t*) ((FRAME0 + frame_num) * NBPG);
    memset(ptptr, 0, NBPG);

    // Return initialized page
    return ptptr;
}
