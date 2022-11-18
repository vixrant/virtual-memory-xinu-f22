/* vmhinit.c - vmhinit */

#include <xinu.h>

/*------------------------------------------------------------------------
 * vmhinit - initialize memory bounds and the virtual free memory list
 *------------------------------------------------------------------------
 */
void vmhinit(void) {
    struct procent *prptr = &proctab[currpid];
    struct memblk *memptr;

    if(prptr->prmeminit == TRUE) {
        return;
    }

    memptr = &prptr->prmemblk; // Header contains information
    memptr->mlength = MAXHSIZE * NBPG;
    memptr->mnext = (struct memblk*) MINVHEAP;
    pdf("Setting allocated for %x \n", VHNUM(MINVHEAP));
    prptr->pralloc[VHNUM(MINVHEAP)] = TRUE;
    memptr = memptr->mnext; // First block is entire free space
    memptr->mlength = MAXHSIZE * NBPG;
    memptr->mnext = NULL;

    prptr->prmeminit = TRUE;
}
