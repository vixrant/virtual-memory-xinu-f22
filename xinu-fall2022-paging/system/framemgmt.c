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
inline void __remove_from_used_ll(fidx16 frame_num) {
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

// Dequeue from E1's linked list in FIFO
inline fidx16 __dequeue_from_used_ll() {
    // List is empty
    if(frheadE1 == SYSERR) {
        return SYSERR;
    }

    // Front of queue
    fidx16 retval = frheadE1;

    // Remove from list
    __remove_from_used_ll(retval);

    return retval;
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
 *  allocaframe  -  Given an index in inverted page table,
 *                  allocate that frame for given pid
 *------------------------------------------------------------------------
 */
syscall allocaframe(
    fidx16 frame_num,
    pid32 pid,
    uint32 page
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
    frptr->fr_page = page;

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


/*------------------------------------------------------------------------
 *  swapframe -  Swap a frame f in E1 with frame g in E2
 *               Performs O(1) extra space swap
 *------------------------------------------------------------------------
 */
syscall swapframe(
    fidx16 f, /* Frame number in E1 */
    fidx16 g  /* Frame number in E2 */
) {
    frame_t *fptr = &invpt[INIDX(f)];
    frame_t *gptr = &invpt[INIDX(g)];

    // TODO: Add range checks

    // 1. XOR swap all bytes of f and g
    uint16 i;
    char *faddr = (char*) (f * NBPG);
    char *gaddr = (char*) (g * NBPG);
    for(i=0 ; i<NBPG ; i++) {
        faddr[i] ^= gaddr[i];
        gaddr[i] ^= faddr[i];
        faddr[i] ^= gaddr[i];
    }

    // 2. Mark f's PTE as swapped, absent
    pt_t *fpte = getpte(fptr->fr_page * NBPG);
    fpte->pt_swap = 1;
    fpte->pt_pres = 0;

    // 3. Mark g's PTE as unswapped, present
    pt_t *gpte = getpte(gptr->fr_page * NBPG);
    gpte->pt_swap = 0;
    gpte->pt_pres = 1;

    return OK;
}

/*------------------------------------------------------------------------
 *  evictframe -  Evicts the frame at front of FIFO queue of E1 to E2
 *------------------------------------------------------------------------
 */
syscall evictframe() {
    // 1. Get destination frame in E2
    fidx16 dest = getfreeframe(REGION_E2);
    if(dest == SYSERR) {
        log_bs("evictframe - no free frames in E2 \n");
        return SYSERR;
    }

    // 2. Get source frame in E1 from FIFO list
    fidx16 victim = __dequeue_from_used_ll();
    if(victim == SYSERR) {
        log_bs("evictframe - no frames to evict \n");
        return SYSERR;
    }

    // 3. Copy destination <- source
    uint16 i;
    char *vaddr = (char*) (victim * NBPG);
    char *daddr = (char*) (dest * NBPG);
    for(i=0 ; i<NBPG ; i++) {
        daddr[i] = vaddr[i];
    }

    // 4. Mark victim's PTE as swapped, absent
    frame_t *vptr = &invpt[INIDX(victim)];
    pt_t *vpte = getpte(vptr->fr_page * NBPG);
    vpte->pt_swap = 1;
    vpte->pt_pres = 0;

    // 5. Deallocated victim frame
    deallocaframe(victim);

    return OK;
}
