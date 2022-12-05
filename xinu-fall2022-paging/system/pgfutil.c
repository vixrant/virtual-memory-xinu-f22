/* pgfutil.c - assignframe, swapframe, evictframe */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  mapnewframe
 *
 *  Assumptions:
 *  - E1: Must have free frame
 *  - No assumptions about previous data
 *
 *  Actions
 *  - Marks E1 free frame as occupied to current process
 *  - Maps the E1 free frame to faulting page
 *------------------------------------------------------------------------
 */
fidx16 mapfreeframe() {
    log_bs("- Mapping new E1 frame to faulting address \n");
    fidx16 frame_num = SYSERR;
    pt_t *pte = getpte(pgfaddr);

    // 1. Get a new frame
    frame_num = getfreeframe(REGION_E1);
    if(frame_num == SYSERR) {
        return SYSERR;
    }

    // 2. Occupy frame
    invtakeframe(frame_num, currpid, pte);
    log_bs("-- New frame %d is now occupied by %d \n", frame_num, currpid);

    // 3. Map page to frame
    log_bs("-- Change PTE from 0x%08x ", *pte);
    pte->pt_pres  = 1;
    pte->pt_write = 1;
    pte->pt_swap  = 0;
    pte->pt_base  = frame_num;
    log_bs("to 0x%08x \n", *pte);

    return frame_num;
}


/*------------------------------------------------------------------------
 *  restoreframe
 *
 *  Assumptions:
 *  - E1: Must have free frame
 *  - E2: Must have backed-up frame
 *
 *  Actions:
 *  - Gets E2 frame that contains faulting page's old data
 *  - Map a free frame in E1
 *  - Copies E2 frame data into temporory storage
 *  - Frees the E2 frame
 *------------------------------------------------------------------------
 */
fidx16 restoreframe() {
    log_bs("- Restoring old E2 frame of faulting address \n");

    // 1. Get backup frame in E2
    pt_t *pte = getpte(pgfaddr);
    fidx16 src = pte->pt_base;

    // 2. Get free frame in E1
    fidx16 dest = mapfreeframe();
    if(dest == SYSERR) {
        log_bs("- Restoreframe failed in mapping free frame in E1 \n");
        return SYSERR;
    }
    log_bs("-- Restoring %d -> %d \n", src, dest);

    // 3. Copy all bytes of E2 into E1
    uint16 i;
    unsigned char *saddr = (unsigned char*) (src * NBPG);
    unsigned char *daddr = (unsigned char*) (dest * NBPG);
    for(i=0 ; i<NBPG ; i++) {
        daddr[i] = saddr[i];
    }
    log_bs("-- Copied destination 0x%08x <-- source 0x%08x \n", daddr, saddr);

    // 4. Give up backup frame
    invfreeframe(src);

    return OK;
}


/*------------------------------------------------------------------------
 *  evictframe
 *
 *  Assumptions:
 *  - E1: Must have used frame (owner = victim)
 *  - E2: Must have free frame
 *
 *  Actions:
 *  - Marks E2 free frame as occupied to victim process
 *  - Moves E1 frame data to E2 frame data
 *  - Frees the E1 frame
 *------------------------------------------------------------------------
 */
syscall evictframe() {
    log_bs("- Evicting an E1 frame to E2 \n");

    // 1. Get source frame in E1 from FIFO list
    fidx16 victim = getusedframeE1();
    if(victim == SYSERR) {
        log_bs("-- No frames to evict \n");
        return SYSERR;
    }

    // 2. Get destination frame in E2
    fidx16 dest = getfreeframe(REGION_E2);
    if(dest == SYSERR) {
        log_bs("-- No free frames in E2 \n");
        return SYSERR;
    }

    frame_t *vptr = &invpt[INIDX(victim)];
    log_bs("-- Selected victim %d --> destination %d \n", victim, dest);

    // 3. Victim occpies E2 frame
    invtakeframe(dest, vptr->fr_pid, vptr->fr_pte);

    // 4. Copy victim -> destination
    uint16 i;
    unsigned char *vaddr = (unsigned char*) (victim * NBPG);
    unsigned char *daddr = (unsigned char*) (dest * NBPG);
    for(i=0 ; i<NBPG ; i++) {
        daddr[i] = vaddr[i];
    }
    log_bs("-- Copied destination 0x%08x <-- victim 0x%08x \n", daddr, vaddr);

    // 5. Mark victim's PTE as swapped, absent, new location
    pt_t *vpte = vptr->fr_pte;
    if(vpte != FR_PTEUNUSED) {
        log_bs("-- Changed PTE from 0x%08x ", *vpte);
        vpte->pt_pres  = 0;
        vpte->pt_write = 0;
        vpte->pt_swap  = 1;
        vpte->pt_base  = dest;
        log_bs("to 0x%08x \n", *vpte);
    }

    // 6. Victim gives up E1 frame
    invfreeframe(victim);

    return OK;
}


/*------------------------------------------------------------------------
 *  swapframe
 *
 *  Assumptions:
 *  - E1: Must have used frame
 *  - E2: Must have backed-up frame
 *
 *  Actions:
 *  - Gets E2 frame that contains faulting page's old data
 *  - Copies E2 frame data into temporory storage
 *  - Frees the E2 frame
 *  - Evicts an E1 frame to now free location in E2
 *  - Maps now free location in E1 to faulting page
 *  - Copies temporary storage data into E1 frame
 *------------------------------------------------------------------------
 */
syscall swapframe() {
    log_pgf("- Swap an E1 frame with old E2 frame of faulting address \n");

    fidx16 frame_num; // Frame numbers in E2 and E1
    uint16 i; // Iterator
    static char temp[NBPG]; // Temperory store

    // 1. Copy all bytes of E2 into temp storage
    pt_t *pte = getpte(pgfaddr);
    frame_num = pte->pt_base;
    char *saddr = (char*) (frame_num * NBPG); // E2 source
    for(i=0 ; i<NBPG ; i++) {
        temp[i] = saddr[i];
    }
    log_bs("-- Copied temp <-- source 0x%08x \n", saddr);

    // 2. Give up swapped E2 frame
    invfreeframe(frame_num);

    // 3. Evict a frame to deallocated E2 frame
    evictframe();

    // 4. Map a new frame
    frame_num = mapfreeframe(); // E1 frame id

    // 5. Copy all bytes of temp storage into new E1 frame
    char *daddr = (char*) (frame_num * NBPG); // E1 destination
    for(i=0 ; i<NBPG ; i++) {
        daddr[i] = temp[i];
    }
    log_bs("-- Copied destination 0x%08x <-- temp \n", daddr);

    return OK;
}
