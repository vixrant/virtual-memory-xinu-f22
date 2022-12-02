/* framemgmt.c - init_invpt,
 *               hasfreeframe, hasusedframeE1,
 *               getfreeframe, getusedframeE1,
 *               invtakeframe, invfreeframe
 */

#include <xinu.h>

/* Inverted page table */
frame_t invpt[NFRAMES];
fidx16 frheadE1 = SYSERR;
fidx16 frtailE1 = SYSERR;

fidx16 frstackD[NFRAMES_D];
int16  frspD;

fidx16 frstackE1[NFRAMES_E1];
int16  frspE1;

fidx16 frstackE2[NFRAMES_E2];
int16  frspE2;

// Remove frame from E1's linked list
inline void __remove_from_used_ll(fidx16 frame_num) {
    frame_t *frptr = &invpt[INIDX(frame_num)];

    if(frheadE1 == frame_num && frtailE1 == frame_num) {
        // Only one node in linked list
        frheadE1 = frtailE1 = SYSERR;
        log_bs("invfreeframe %d - deleted linked list \n", frame_num);
    } else if(frheadE1 == frame_num) {
        // Invfreeting head
        frheadE1 = frptr->fr_next->fr_idx;
        log_bs("invfreeframe %d - moved head to %d \n", frame_num, frheadE1);
        frptr->fr_next->fr_prev = NULL;
    } else if(frtailE1 == frame_num) {
        // Invfreeting tail
        frtailE1 = frptr->fr_prev->fr_idx;
        log_bs("invfreeframe %d - moved tail to %d \n", frame_num, frtailE1);
        frptr->fr_prev->fr_next = NULL;
    } else {
        // Invfreeting a middle node
        frptr->fr_prev->fr_next = frptr->fr_next;
        frptr->fr_next->fr_prev = frptr->fr_prev;
        log_bs("invfreeframe %d - deleted middle node \n", frame_num);
    }

    frptr->fr_next = frptr->fr_prev = NULL;
}

// Add frame to E1's linked list in FIFO
inline void __insert_to_used_ll(fidx16 frame_num) {
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
 *  init_intpt  -  Initialize inverted page table
 *------------------------------------------------------------------------
 */
void init_invpt() {
    uint16 i; /* Initialization loop iterator */

    for(i=0 ; i<NFRAMES ; i++) {
        invpt[i].fr_idx = i + FRAME0; // list indexes
        invpt[i].fr_state = FR_FREE; // all frames free
        invpt[i].fr_next = invpt[i].fr_prev = NULL; // linked list
    }

    for(i=0 ; i<NFRAMES_D  ; i++) // set up index stacks
        frstackD[i] = i + FRAME0_D;

    for(i=0 ; i<NFRAMES_E1 ; i++)
        frstackE1[i] = i + FRAME0_E1;

    for(i=0 ; i<NFRAMES_E2 ; i++)
        frstackE2[i] = i + FRAME0_E2;

    frspD = frspE1 = frspE2 = 0; // stack pointers
}


/*------------------------------------------------------------------------
 *  hasfreeframe  -  Checks if region has free frames
 *------------------------------------------------------------------------
 */
bool8 hasfreeframe(region r) {
    switch(r) {
        case REGION_D:
            return frspD != NFRAMES_D;
            break;

        case REGION_E1:
            return frspE1 != NFRAMES_E1;
            break;

        case REGION_E2:
            return frspE2 != NFRAMES_E2;
            break;
    }

    return SYSERR;
}


/*------------------------------------------------------------------------
 *  hasusedframeE1  -  Checks if region E1 has used frames in FIFO
 *------------------------------------------------------------------------
 */
bool8 hasusedframeE1() {
    return frheadE1 != SYSERR;
}


/*------------------------------------------------------------------------
 *  getfreeframe  -  Return frame in region passed
 *------------------------------------------------------------------------
 */
fidx16 getfreeframe(region r) {
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
 *  getusedframeE1  -  Return frame in region E1
 *------------------------------------------------------------------------
 */
fidx16 getusedframeE1() {
    // List is empty
    if(frheadE1 == SYSERR) {
        return SYSERR;
    }

    // Front of queue
    fidx16 retval = frheadE1;

    // Remove from list
    __remove_from_used_ll(retval);

    log_fr("getusedframe - returning %d \n", r, retval);
    return retval;
}


/*------------------------------------------------------------------------
 *  invtakeframe  -  Given an index in inverted page table,
 *                   allocate that frame for given pid
 *------------------------------------------------------------------------
 */
syscall invtakeframe(
    fidx16 frame_num,
    pid32 pid,
    pt_t *pte
) {
    intmask mask;
    frame_t *frptr = &invpt[INIDX(frame_num)];

    mask = disable();

    if(frame_num < 0 || frame_num >= NFRAMES) {
        log_fr("invtakeframe - frame %d out of bounds \n", frame_num);
        restore(mask);
        return SYSERR;
    }

    if(frptr->fr_state != FR_FREE) {
        log_fr("invtakeframe - frame %d is occupied by %d \n", frame_num, frptr->fr_pid);
        restore(mask);
        return SYSERR;
    }

    // Set invpt fields
    frptr->fr_state = FR_USED;
    frptr->fr_pid = pid;
    frptr->fr_pte = pte;

    // Add to used linked list
    if(FRAME0_E1 <= frame_num && frame_num < FRAME0_F) {
        log_bs("invtakeframe - detected frame %d in E1 \n", frame_num);
        __insert_to_used_ll(frame_num);
    }

    log_fr("invtakeframe - allocated frame %d to %d \n", frame_num, pid);
    restore(mask);
    return OK;
}


/*------------------------------------------------------------------------
 *  invfreeframe  -  Given an index in inverted page table,
 *                   invfreete the frame and add it back to stack
 *------------------------------------------------------------------------
 */
syscall invfreeframe(
    fidx16 frame_num
) {
    intmask mask;
    frame_t *frptr = &invpt[INIDX(frame_num)];

    mask = disable();

    if(frptr->fr_state == FR_FREE) {
        // No action required
        restore(mask);
        return OK;
    }

    // Try to push to correct stack
    if(frame_num < FRAME0_E1) {
        if(frspD == 0) {
            log_fr("invfreeframe - cannot push %d into region D because stack is full \n", frame_num);
            restore(mask);
            return SYSERR;
        }

        log_fr("dellocaframe - pushed to D \n");
        frstackD[--frspD] = frame_num;
    } else if(frame_num < FRAME0_E2) {
        if(frspE1 == 0) {
            log_fr("invfreeframe - cannot push %d into region E1 because stack is full \n", frame_num);
            restore(mask);
            return SYSERR;
        }

        log_fr("dellocaframe - pushed to E1 \n");
        frstackE1[--frspE1] = frame_num;

        // Remove from used link list
        __remove_from_used_ll(frame_num);
    } else if(frame_num < FRAME0_F) {
        if(frspE2 == 0) {
            log_fr("invfreeframe - cannot push %d into region E2 because stack is full \n", frame_num);
            restore(mask);
            return SYSERR;
        }

        log_fr("invfreeframe - pushed to E2 \n");
        frstackE2[--frspE2] = frame_num;
    } else {
        log_fr("invfreeframe - frame %d out of bound \n", frame_num);
        restore(mask);
        return SYSERR;
    }

    // Set to free
    frptr->fr_state = FR_FREE;

    log_fr("invfreeframe - deallocated frame %d \n", frame_num);
    restore(mask);
    return OK;
}
