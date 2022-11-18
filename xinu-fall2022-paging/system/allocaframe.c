/* allocaframe.c - allocaframe */

#include <xinu.h>

syscall allocaframe(int16 frame_num, pid32 pid) {
    frame_t *frptr; /* Pointer to frame in inverted page table */
    frptr = &invpt[frame_num];

    if(frptr->fr_state != FR_FREE) {
        pdf("allocaframe %d - frame is occupied by %d \n", frame_num, frptr->fr_pid);
        return SYSERR;
    }

    frptr->fr_state = FR_USEDT;
    frptr->fr_pid = pid;
    return OK;
}
