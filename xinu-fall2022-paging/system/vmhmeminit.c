#include <xinu.h>

/*------------------------------------------------------------------------
 * vmhmeminit - initialize memory bounds and the free memory list
 *------------------------------------------------------------------------
 */
void    vmhmeminit(struct memblk *prmemlist) {
    struct  memblk  *memptr;    /* Ptr to memory block      */

    /* Initialize the free list */
    memptr = prmemlist;
    memptr->mnext = (struct memblk *)NULL;
    memptr->mlength = 0;

    /* Initialize the memory counters */
    /*    Virtual Heap starts at the end of Xinu image */

    memptr->mlength = MAXHSIZE;
    memptr->mnext = (struct memblk *) 0x01000000;

    memptr = memptr->mnext;
    memptr->mnext = NULL;
    memptr->mlength = (uint32)maxheap - (uint32)minheap;
}
