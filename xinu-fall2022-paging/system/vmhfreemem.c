/* vmhfreemem.c - vmhfreemem */

#include <xinu.h>

// 1. Set m pages to unallocated
// 2. Set the PTE for the page as absent
// 3. Add frame back to its stack
static inline void __set_pages_deallocated(
    uint16 msize,
    char *blkaddr
) {
    struct procent *prptr = &proctab[currpid];
    uint32 i;

    for(i=0 ; i<msize ; i++) {
        uint32 addr = ((uint32) blkaddr) + (i * NBPG);
        log_fr("vmhfreemem - deallocating 0x%08x \n", addr);
        // Set unallocated in process cell
        prptr->pralloc[VHNUM(addr)] = FALSE;
        // Get PTE
        pt_t *pte = getpte((char*) addr);
        if(pte->pt_pres == 1) {
            // Mark as absent
            pte->pt_pres = 0;
            log_fr("vmhfreemem - marked PTE %d absent \n", *pte);
            // Get frame mapping
            fidx16 frame_idx = pte->pt_base - FRAME0;
            // Mark the frame as free in inverted page table
            // Add back to its stack
            if(invpt[frame_idx].fr_pid != currpid) {
                kprintf("Error in code: Deallocating frame belonging to %d that does not belong to %d \n", invpt[frame_idx].fr_pid, currpid);
            }
            deallocaframe(frame_idx);
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
        prev->mlength += nbytes;
        block = prev;
    } else {            /* Link into list as new node   */
        block->mnext = next;
        block->mlength = nbytes;
        prev->mnext = block;
    }

    /* Coalesce with next block if adjacent */

    if (((uint32) block + block->mlength) == (uint32) next) {
        block->mlength += next->mlength;
        block->mnext = next->mnext;
    }

    // Update kernel data structures
    __set_pages_deallocated(msize, blkaddr);

    restore(mask);
    return OK;
}
