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
    fidx16 frame_num = SYSERR;
    pt_t *pte = getpte(pgfaddr);

    // 1. Get a new frame
    frame_num = getfreeframe(REGION_E1);
    if(frame_num == SYSERR) {
        return SYSERR;
    }

    // 2. Occupy frame
    invtakeframe(frame_num, currpid, pte);

    // 3. Map page to frame
    log_pgf("- Mapping %d -> %d \n", PGNUM(pgfaddr), frame_num);
    pte->pt_pres  = 1;
    pte->pt_write = 1;
    pte->pt_swap  = 0;
    pte->pt_base  = frame_num;

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
 *  - Occupies a free frame in E1
 *  - Copies E2 frame data into temporory storage
 *  - Frees the E2 frame
 *  - Map faulting page's PTE to E1 frame
 *------------------------------------------------------------------------
 */
fidx16 restoreframe() {
    // 1. Get free frame in E1
    fidx16 dest = getfreeframe(REGION_E1);
    if(dest == SYSERR) {
        log_bs("restoreframe - no free frame in E1 \n");
        return SYSERR;
    }

    // 2. Get backup frame in E2
    pt_t *pte = getpte(pgfaddr);
    fidx16 src = pte->pt_base;

    // 3. Occupy destination frame
    invtakeframe(dest, currpid, pte);
    pte = getpte(pgfaddr);

    // 4. Copy all bytes of E2 into E1
    uint16 i;
    char *saddr = (char*) (src * NBPG);
    char *daddr = (char*) (dest * NBPG);
    for(i=0 ; i<NBPG ; i++) {
        daddr[i] = saddr[i];
    }

    // 5. Give up backup frame
    invfreeframe(src);

    // 6. Mark faulting page's PTE as unswapped, present, new location
    pte->pt_pres  = 1;
    pte->pt_write = 1;
    pte->pt_swap  = 0;
    pte->pt_base  = dest;

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
    // 1. Get destination frame in E2
    fidx16 dest = getfreeframe(REGION_E2);
    if(dest == SYSERR) {
        log_bs("evictframe - no free frames in E2 \n");
        return SYSERR;
    }

    // 2. Get source frame in E1 from FIFO list
    fidx16 victim = getusedframeE1();
    if(victim == SYSERR) {
        log_bs("evictframe - no frames to evict \n");
        return SYSERR;
    }

    frame_t *vptr = &invpt[INIDX(victim)];

    // 3. Victim occpies E2 frame
    invtakeframe(dest, vptr->fr_pid, vptr->fr_pte);

    // 4. Copy victim -> destination
    uint16 i;
    char *vaddr = (char*) (victim * NBPG);
    char *daddr = (char*) (dest * NBPG);
    for(i=0 ; i<NBPG ; i++) {
        daddr[i] = vaddr[i];
    }

    // 5. Mark victim's PTE as swapped, absent, new location
    pt_t *vpte = vptr->fr_pte;
    if(vpte != (pt_t*) FR_PGUNUSED) {
        vpte->pt_pres  = 0;
        vpte->pt_write = 0;
        vpte->pt_swap  = 1;
        vpte->pt_base  = dest;
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
    fidx16 frame_num; // Frame numbers in E2 and E1
    char *addr; // Base addresses in E2 and E1
    uint16 i; // Iterator
    char temp[NBPG]; // Temperory store

    // 1. Copy all bytes of E2 into temp storage
    pt_t *pte = getpte(pgfaddr);
    frame_num = pte->pt_base;
    addr = (char*) (frame_num * NBPG);
    for(i=0 ; i<NBPG ; i++) {
        temp[i] = addr[i];
    }

    // 2. Give up swapped E2 frame
    invfreeframe(frame_num);

    // 3. Evict a frame to deallocated E2 frame
    evictframe();

    // 4. Map a new frame
    frame_num = mapfreeframe(); // E1 frame id

    // 5. Copy all bytes of temp storage into new E1 frame
    addr = (char*) (frame_num * NBPG); // E1 frame base address
    for(i=0 ; i<NBPG ; i++) {
        addr[i] = temp[i];
    }

    return OK;
}
