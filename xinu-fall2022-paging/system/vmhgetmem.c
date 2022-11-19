/* vmhgetmem.c - vmhgetmem */

#include <xinu.h>

// Set m pages all to allocated
static inline void __set_pages_allocated(
    uint16 msize,
    struct memblk *blkaddr
) {
    struct  procent *prptr = &proctab[currpid];
    uint32 i;

    for(i=0 ; i<msize ; i++) {
        uint32 addr = ((uint32) blkaddr) + (i * NBPG);
        log_fr("vmhgetmem - allocating 0x%08x \n", addr);
        // Set allocated in process cell
        prptr->pralloc[VHNUM(addr)] = TRUE;
    }
}

/*------------------------------------------------------------------------
 *  vhmgetmem - Allocate vheap storage, returning lowest word address
 *------------------------------------------------------------------------
 */
char *vmhgetmem(
      uint16    msize      /* Size of memory requested in pages */
    )
{
    intmask     mask;           /* Saved interrupt mask         */
    struct  memblk  *prev, *curr, *leftover;
    struct  procent *prptr = &proctab[currpid];
    uint32 nbytes = msize * NBPG; // Granuarity change

    vmhinit(); // Initialize vheap if not done already

    mask = disable();
    if (nbytes == 0 || prptr->prmemblk.mlength == 0) {
        restore(mask);
        return (char *)SYSERR;
    }

    prev = &prptr->prmemblk;
    curr = prev->mnext;
    while (curr != NULL) {          /* Search free list     */

        if (curr->mlength == nbytes) {  /* Block is exact match     */
            prev->mnext = curr->mnext;
            prptr->prmemblk.mlength -= nbytes;
            __set_pages_allocated(msize, curr);
            restore(mask);
            return (char *)(curr);

        } else if (curr->mlength > nbytes) { /* Split big block     */
            leftover = (struct memblk *)((uint32) curr + nbytes);
            prptr->pralloc[VHNUM((uint32) leftover)] = TRUE;
            prev->mnext = leftover;
            leftover->mnext = curr->mnext;
            leftover->mlength = curr->mlength - nbytes;
            prptr->prmemblk.mlength -= nbytes;
            __set_pages_allocated(msize, curr);
            restore(mask);
            return (char *)(curr);
        } else {            /* Move to next block   */
            prev = curr;
            curr = curr->mnext;
        }
    }
    restore(mask);
    return (char *)SYSERR;
}
