/* vmhgetmem.c - vmhgetmem */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  vhmgetmem  -  Allocate vheap storage, returning lowest word address
 *------------------------------------------------------------------------
 */
char *vmhgetmem(
      uint16    msize      /* Size of memory requested in pages */
    )
{
    intmask     mask;           /* Saved interrupt mask         */
    struct  memblk  *prev, *curr, *leftover;
    struct  procent *prptr;
    uint32 nbytes = msize * NBPG; // Granuarity change

    mask = disable();
    if (nbytes == 0) {
        restore(mask);
        return (char *)SYSERR;
    }

    prptr = &proctab[currpid];
    prev = &prptr->prmemblk;
    curr = prev->mnext;
    while (curr != NULL) {          /* Search free list     */

        if (curr->mlength == nbytes) {  /* Block is exact match     */
            prev->mnext = curr->mnext;
            prptr->prmemblk.mlength -= nbytes;
            restore(mask);
            return (char *)(curr);

        } else if (curr->mlength > nbytes) { /* Split big block     */
            leftover = (struct memblk *)((uint32) curr + nbytes);
            prev->mnext = leftover;
            leftover->mnext = curr->mnext;
            leftover->mlength = curr->mlength - nbytes;
            prptr->prmemblk.mlength -= nbytes;
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
