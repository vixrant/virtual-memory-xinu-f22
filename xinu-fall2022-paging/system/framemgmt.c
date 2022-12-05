/* framemgmt.c - init_invpt,
 *               hasfreeframe, hasusedframeE1,
 *               getfreeframe, getusedframeE1,
 *               invtakeframe, invfreeframe
 */

#include <xinu.h>

/* Inverted page table */
frame_t invpt[NFRAMES];
frame_t *evicthead = (frame_t*) NULL;
frame_t *evicttail = (frame_t*) NULL;

fidx16 frstackD[NFRAMES_D];
int16  frspD;

fidx16 frstackE1[NFRAMES_E1];
int16  frspE1;

fidx16 frstackE2[NFRAMES_E2];
int16  frspE2;

// Remove frame from E1's linked list
inline void __remove_from_used_ll(fidx16 frame_num) {
    frame_t *frptr = &invpt[INIDX(frame_num)];

     /* Remove the frame from the list */
    if (frptr->fr_prev != NULL) {
        frptr->fr_prev->fr_next = frptr->fr_next;
    }

    if (frptr->fr_next != NULL) {
        frptr->fr_next->fr_prev = frptr->fr_prev;
    }

    log_fr("invfreeframe %d - removed from linked list \n", frame_num);
    
    /* Update the head and tail pointers if necessary */
    if (frptr == evicthead) {
        evicthead = frptr->fr_next;
        log_fr("invfreeframe %d - moved evicthead to \n", frame_num, evicthead->fr_idx);
    }

    if (frptr == evicttail) {
        evicttail = frptr->fr_prev;
        log_fr("invfreeframe %d - moved evicttail to \n", frame_num, evicttail->fr_idx);
    }

    frptr->fr_next = frptr->fr_prev = NULL;
}

// Add frame to E1's linked list in FIFO
inline void __insert_to_used_ll(fidx16 frame_num) {
    frame_t *frptr = &invpt[INIDX(frame_num)];
    log_fr("invtakeframe %d - inserting into linked list \n", frame_num);

    /* Insert the frame at the tail of the list */
    frptr->fr_prev = evicttail;
    frptr->fr_next = NULL;
    if (evicttail != NULL) {
        evicttail->fr_next = frptr;
    }
    evicttail = frptr;
    log_fr("invtakeframe %d - added frame %d to end of ULL \n", frame_num, evicttail);
    
    /* Update the head of the list if necessary */
    if (evicthead == NULL) {
        evicthead = frptr;
        log_fr("invtakeframe %d - made frame %d as head of ULL \n", frame_num, evicthead);
    }
}


/*------------------------------------------------------------------------
 *  fidxtoregion  -  Frame to Region mapping
 *------------------------------------------------------------------------
 */
region fidxtoregion(fidx16 frame_num) {
    if(FRAME0_D <= frame_num && frame_num < FRAME0_E1) {
        return REGION_D;
    }

    if(FRAME0_E1 <= frame_num && frame_num < FRAME0_E2) {
        return REGION_E1;
    }

    if(FRAME0_E2 <= frame_num && frame_num < FRAME0_F) {
        return REGION_E2;
    }

    return SYSERR;
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
            return (frspD < NFRAMES_D);
        
        case REGION_E1:
            return (frspE1 < NFRAMES_E1);
        
        case REGION_E2:
            return (frspE2 < NFRAMES_E2);
    }

    return SYSERR;
}


/*------------------------------------------------------------------------
 *  hasusedframeE1  -  Checks if region E1 has used frames in FIFO
 *------------------------------------------------------------------------
 */
bool8 hasusedframeE1() {
    return evicthead != NULL;
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
    if(evicthead == NULL) {
        return SYSERR;
    }

    // Front of queue
    fidx16 retval = evicthead->fr_idx;

    log_fr("getusedframe - returning %d \n", retval);
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
    if(fidxtoregion(frame_num) == REGION_E1) {
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
syscall invfreeframe(fidx16 frame_num) {
    intmask mask;
    frame_t *frptr = &invpt[INIDX(frame_num)];

    mask = disable();

    if(frptr->fr_state == FR_FREE) {
        // No action required
        log_fr("invfreeframe - frame %d is already free \n", frame_num);
        restore(mask);
        return OK;
    }

    // Try to push to correct stack
    switch(fidxtoregion(frame_num)) {
        case REGION_D:
            if(frspD == 0) {
                log_fr("invfreeframe - cannot push %d into region D because stack is full \n", frame_num);
                restore(mask);
                return SYSERR;
            }

            log_fr("dellocaframe - pushed to D \n");
            frstackD[--frspD] = frame_num;
            break;

        case REGION_E1:
            if(frspE1 == 0) {
                log_fr("invfreeframe - cannot push %d into region E1 because stack is full \n", frame_num);
                restore(mask);
                return SYSERR;
            }

            log_fr("dellocaframe - pushed to E1 \n");
            frstackE1[--frspE1] = frame_num;

            // Remove from used link list
            __remove_from_used_ll(frame_num);
            break;

        case REGION_E2:
            if(frspE2 == 0) {
                log_fr("invfreeframe - cannot push %d into region E2 because stack is full \n", frame_num);
                restore(mask);
                return SYSERR;
            }

            log_fr("invfreeframe - pushed to E2 \n");
            frstackE2[--frspE2] = frame_num;
            break;

        default:
            log_fr("invfreeframe - frame %d out of bound \n", frame_num);
            restore(mask);
            return SYSERR;
    }

    // Set to free
    frptr->fr_state = FR_FREE;

    log_fr("invfreeframe %d - deallocated frame \n", frame_num);
    restore(mask);
    return OK;
}
