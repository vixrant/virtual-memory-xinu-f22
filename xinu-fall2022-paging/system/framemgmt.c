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
 *  allocaframe  -  Given an index in inverted page table,
 *                  allocate that frame for given pid
 *------------------------------------------------------------------------
 */
syscall allocaframe(
    fidx16 frame_num,
    pid32 pid
) {
    frame_t *frptr; /* Pointer to frame in inverted page table */
    frptr = &invpt[frame_num - FRAME0];

    if(frame_num < 0 || frame_num >= NFRAMES) {
        return SYSERR;
    }

    if(frptr->fr_state != FR_FREE) {
        log_fr("allocaframe - frame %d is occupied by %d \n", frame_num, frptr->fr_pid);
        return SYSERR;
    }

    frptr->fr_state = FR_USED;
    frptr->fr_pid = pid;

    log_fr("allocaframe - allocated frame %d to %d \n", frame_num, pid);
    return OK;
}

/*------------------------------------------------------------------------
 *  deallocaframe  -  Given an index in inverted page table,
 *                    deallocate the frame and add it back to stack
 *------------------------------------------------------------------------
 */
syscall deallocaframe(
    fidx16 frame_num
) {
    frame_t *frptr; /* Pointer to frame in inverted page table */
    frptr = &invpt[frame_num - FRAME0];

    if(frame_num < FRAME0_E1) {
        if(frspD == 0) {
            log_fr("deallocaframe - cannot push %d into region D because stack is full \n", frame_num);
            return SYSERR;
        }

        log_fr("dellocaframe - pushed to D \n");
        frstackD[--frspD] = frame_num;
    } else if(frame_num < FRAME0_E2) {
        if(frspE1 == 0) {
            log_fr("deallocaframe - cannot push %d into region E1 because stack is full \n", frame_num);
            return SYSERR;
        }

        log_fr("dellocaframe - pushed to E1 \n");
        frstackE1[--frspE1] = frame_num;
    } else if(frame_num < FRAME0_F) {
        if(frspE2 == 0) {
            log_fr("deallocaframe - cannot push %d into region E2 because stack is full \n", frame_num);
            return SYSERR;
        }

        log_fr("dellocaframe - pushed to E2 \n");
        frstackE2[--frspE2] = frame_num;
    } else {
        log_fr("deallocaframe - frame %d out of bound \n", frame_num);
        return SYSERR;
    }

    frptr->fr_state = FR_FREE;

    log_fr("deallocaframe - deallocated frame %d \n", frame_num);
    return OK;
}
