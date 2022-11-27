/* framemgmt.c - getfreeframe, allocaframe */

#include <xinu.h>

fidx16 frstackD[NFRAMES_D];
int16 frspD = 0;

fidx16 frstackE1[NFRAMES_E1];
int16 frspE1 = 0;

fidx16 frstackE2[NFRAMES_E2];
int16 frspE2 = 0;

fidx16 frheadE1 = SYSERR;
fidx16 frtailE1 = SYSERR;

// Remove frame from E1's linked list
void __remove_from_used_ll(fidx16 frame_num) {
    frame_t *frptr = &invpt[INIDX(frame_num)];

    if(frheadE1 == frame_num && frtailE1 == frame_num) {
        // Only one node in linked list
        frheadE1 = frtailE1 = SYSERR;
        log_bs("deallocaframe %d - deleted linked list \n", frame_num);
    } else if(frheadE1 == frame_num) {
        // Deallocating head
        frheadE1 = frptr->fr_next->fr_idx;
        log_bs("deallocaframe %d - moved head to %d \n", frame_num, frheadE1);
        frptr->fr_next->fr_prev = NULL;
    } else if(frtailE1 == frame_num) {
        // Deallocating tail
        frtailE1 = frptr->fr_prev->fr_idx;
        log_bs("deallocaframe %d - moved tail to %d \n", frame_num, frtailE1);
        frptr->fr_prev->fr_next = NULL;
    } else {
        // Deallocating a middle node
        frptr->fr_prev->fr_next = frptr->fr_next;
        frptr->fr_next->fr_prev = frptr->fr_prev;
        log_bs("deallocaframe %d - deleted middle node \n", frame_num);
    }

    frptr->fr_next = frptr->fr_prev = NULL;
}

// Add frame to E1's linked list in FIFO
void __insert_to_used_ll(fidx16 frame_num) {
    frame_t *frptr = &invpt[INIDX(frame_num)];

    if(frheadE1 == SYSERR) { // DLL is empty
        frptr->fr_next = frptr->fr_prev = NULL;
        frheadE1 = frtailE1 = frame_num; // 0 <- x -> 0
        log_bs("allocaframe - made frame %d as start of ULL \n", frheadE1);
    } else { // DLL has head and tail
        frame_t *frtailptr = &invpt[INIDX(frtailE1)];
        frtailptr->fr_next = frptr; // t -> x
        frptr->fr_prev = frtailptr; // t <- x
        frptr->fr_next = NULL;      // x -> 0
        frtailE1 = frame_num;
        log_bs("allocaframe - added frame %d to end of ULL \n", frtailE1);
    }
}

/*------------------------------------------------------------------------
 *  getfreeframe  -  Return frame in region
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
 *  getusedframe  -  Return frame in region E1 in FIFO manner
 *------------------------------------------------------------------------
 */
fidx16 getusedframe() {
    return frheadE1;
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
    frptr = &invpt[INIDX(frame_num)];

    if(frame_num < 0 || frame_num >= NFRAMES) {
        log_fr("allocaframe - frame %d out of bounds \n", frame_num);
        return SYSERR;
    }

    if(frptr->fr_state != FR_FREE) {
        log_fr("allocaframe - frame %d is occupied by %d \n", frame_num, frptr->fr_pid);
        return SYSERR;
    }

    // Set invpt fields
    frptr->fr_state = FR_USED;
    frptr->fr_pid = pid;

    // Add to used linked list
    if(FRAME0_E1 <= frame_num && frame_num < FRAME0_F) {
        log_bs("allocaframe - detected frame %d in E1 \n", frame_num);
        __insert_to_used_ll(frame_num);
    }

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
    frame_t *frptr = &invpt[INIDX(frame_num)];

    if(frptr->fr_state == FR_FREE) {
        // No action required
        return OK;
    }

    // Try to push to correct stack
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

        // Remove from used link list
        __remove_from_used_ll(frame_num);
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

    // Set to free
    frptr->fr_state = FR_FREE;

    log_fr("deallocaframe - deallocated frame %d \n", frame_num);
    return OK;
}
