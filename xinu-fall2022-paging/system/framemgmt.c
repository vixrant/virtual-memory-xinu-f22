/* framemgmt.c - getfreeframe, allocaframe */

#include <xinu.h>

fidx16 frstackD[NFRAMES_D];
int16 frspD;

fidx16 frstackE1[NFRAMES_E1];
int16 frspE1;

fidx16 frstackE2[NFRAMES_E2];
int16 frspE2;

/*------------------------------------------------------------------------
 *  getfreeframe  -  Return frame in region,
 *                   returning relative frame number with resp to invpt
 *------------------------------------------------------------------------
 */
fidx16 getfreeframe(
    region r
) {
    fidx16 retval = SYSERR;

    // Pop from region's free frame stack
    switch(r) {
        case REGION_D:
            if(frspD != NFRAMES_D)
                retval = frstackD[frspD++];
            break;

        case REGION_E1:
            if(frspE1 != NFRAMES_E1)
                retval = frstackE1[frspE1++];
            break;

        case REGION_E2:
            if(frspE2 != NFRAMES_E2)
                retval = frstackE2[frspE2++];
            break;
    }

    log_fr("getfreeframe %d - returning %d \n", r, retval);
    return retval;
}

/*------------------------------------------------------------------------
 *  deallocaframe  -  Given an index in inverted page table,
 *                    deallocate the frame and add it back to stack
 *------------------------------------------------------------------------
 */
syscall deallocaframe(
    fidx16 frame_idx
) {
    frame_t *frptr; /* Pointer to frame in inverted page table */
    frptr = &invpt[frame_idx];

    if(frame_idx < FRAME0_E1 - FRAME0) {
        if(frspD == 0) {
            log_fr("deallocaframe - cannot push %d into region D because stack is full \n", frame_idx);
            return SYSERR;
        }

        frstackD[--frspD] = frame_idx;
    } else if(frame_idx < FRAME0_E2 - FRAME0) {
        if(frspE1 == 0) {
            log_fr("deallocaframe - cannot push %d into region E1 because stack is full \n", frame_idx);
            return SYSERR;
        }

        frstackE1[--frspE1] = frame_idx;
    } else if(frame_idx < FRAME0_F - FRAME0) {
        if(frspE2 == 0) {
            log_fr("deallocaframe - cannot push %d into region E2 because stack is full \n", frame_idx);
            return SYSERR;
        }

        frstackE2[--frspE2] = frame_idx;
    } else {
        log_fr("deallocaframe - frame %d out of bound \n", frame_idx);
        return SYSERR;
    }

    frptr->fr_state = FR_FREE;

    log_fr("deallocaframe - deallocated frame %d \n", frame_idx);
    return OK;
}


/*------------------------------------------------------------------------
 *  allocaframe  -  Given an index in inverted page table,
 *                  allocate that frame for given pid
 *------------------------------------------------------------------------
 */
syscall allocaframe(
    fidx16 frame_idx,
    pid32 pid
) {
    frame_t *frptr; /* Pointer to frame in inverted page table */
    frptr = &invpt[frame_idx];

    if(frame_idx < 0 || frame_idx >= NFRAMES) {
        return SYSERR;
    }

    if(frptr->fr_state != FR_FREE) {
        log_fr("allocaframe - frame %d is occupied by %d \n", frame_idx, frptr->fr_pid);
        return SYSERR;
    }

    frptr->fr_state = FR_USED;
    frptr->fr_pid = pid;

    log_fr("allocaframe - allocated frame %d to %d \n", frame_idx, pid);
    return OK;
}
