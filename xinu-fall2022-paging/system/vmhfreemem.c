/* vmhfreemem.c - vmhfreemem */

#include <xinu.h>

// 1. Set m pages to unallocated
// 2. Set the PTE for the page as absent
// 3. Add frame back to its stack
static void __set_pages_deallocated(
    uint16 msize,
    char *blkaddr,
    bool8 skipfirst
) {
    struct procent *prptr = &proctab[currpid];
    uint32 i;

    for(i=0 ; i<msize ; i++) {
        uint32 addr = ((uint32) blkaddr) + (i * NBPG);
        log_fr("vmhfreemem - deallocating 0x%08x \n", addr);
        if(i == 0 && skipfirst) {
            log_fr("vmhfreemem - skipping link node \n");
            continue;
        }
        // Set unallocated in process cell
        log_fr("vmhfreemem - setting %d as deallocated \n", VHNUM(addr));
        prptr->pralloc[VHNUM(addr)] = FALSE;
        // Get PTE
        pt_t *pte = getpte((char*) addr);
        if(pte->pt_pres == 1) {
            log_fr("vmhfreemem - 0x%08x maps to frame %d \n", addr, getframenum(addr));
            // Mark as absent
            pte->pt_pres = 0;
            asm volatile("invlpg (%0)" :: "r" (addr) : "memory");
            log_fr("vmhfreemem - marked PTE %d absent \n", *pte);
            // Get frame mapping
            fidx16 frame_num = pte->pt_base;
            // Mark the frame as free in inverted page table
            // Add back to its stack
            if(invpt[frame_num - FRAME0].fr_pid != currpid) {
                kprintf("Error in code: Deallocating frame belonging to %d that does not belong to %d \n", invpt[frame_num - FRAME0].fr_pid, currpid);
            }
            deallocaframe(frame_num);
        }
    }
}

/*---------------------------------------------------------------------------
 *  vmhfreemem  -  Free a memory block, returning the block to the free list
 *---------------------------------------------------------------------------
 */
syscall     vmhfreemem(
      char      *blkaddr,   /* Pointer to memory block  */
      uint16    msize       /* Size of block in pages */
    )
{
    intmask     mask;           /* Saved interrupt mask         */
    struct  memblk  *next, *prev, *block;
    struct  procent *prptr = &proctab[currpid];
    uint32  top;
    uint32 nbytes = msize * NBPG; // Granularity change

    vmhinit(); // Initialize vheap if not done already

    mask = disable();
    if ((nbytes == 0) || ((uint32) blkaddr < MINVHEAP)
              || ((uint32) blkaddr > MAXVHEAP)) {
        restore(mask);
        log_mem("vmhfreemem - Out of bounds?? \n");
        return SYSERR;
    }

    block = (struct memblk *)blkaddr;

    prev = &prptr->prmemblk;         /* Walk along free list     */
    next = prev->mnext;
    while ((next != NULL) && (next < block)) {
        prev = next;
        next = next->mnext;
    }

    if (prev == &prptr->prmemblk) {         /* Compute top of previous block*/
        top = (uint32) NULL;
    } else {
        top = (uint32) prev + prev->mlength;
    }

    /* Ensure new block does not overlap previous or next blocks    */

    if (((prev != &prptr->prmemblk) && (uint32) block < top)
        || ((next != NULL)  && (uint32) block+nbytes>(uint32)next)) {
        restore(mask);
        log_mem("vmhfreemem - overlaps previous or next blocks?? \n");
        return SYSERR;
    }

    prptr->prmemblk.mlength += nbytes;

    /* Either coalesce with previous block or add to free list */

    if (top == (uint32) block) {    /* Coalesce with previous block     */
        log_mem("vmhfreemem - coalesce \n");
        prev->mlength += nbytes;
        block = prev;
        __set_pages_deallocated(msize, blkaddr, FALSE);
    } else {            /* Link into list as new node   */
        log_mem("vmhfreemem - new node \n");
        block->mnext = next;
        block->mlength = nbytes;
        prev->mnext = block;
        __set_pages_deallocated(msize, blkaddr, TRUE);
    }

    /* Coalesce with next block if adjacent */

    if (((uint32) block + block->mlength) == (uint32) next) {
        block->mlength += next->mlength;
        block->mnext = next->mnext;
    }

    restore(mask);
    return OK;
}
