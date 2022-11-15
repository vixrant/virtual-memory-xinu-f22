/* vmhgetmem.c - vmhgetmem */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  vhmgetmem  -  Allocate vheap storage, returning lowest word address
 *------------------------------------------------------------------------
 */
char *vmhgetmem(
      uint16    nbytes      /* Size of memory requested     */
    )
{
    /* intmask     mask;           /\* Saved interrupt mask         *\/ */
    /* struct  memblk  *prev, *curr, *leftover; */
    /* struct  procent *prptr; */

    /* mask = disable(); */
    /* if (nbytes == 0) { */
    /*     restore(mask); */
    /*     return (char *)SYSERR; */
    /* } */

    /* prptr = &proctab[currpid]; */

    /* nbytes = (uint32) roundmb(nbytes);  /\* Use memblk multiples     *\/ */

    /* prev = &prptr->prmemlist; */
    /* curr = prev->mnext; */
    /* while (curr != NULL) {          /\* Search free list     *\/ */

    /*     if (curr->mlength == nbytes) {  /\* Block is exact match     *\/ */
    /*         prev->mnext = curr->mnext; */
    /*         prptr->prmemlist.mlength -= nbytes; */
    /*         restore(mask); */
    /*         return (char *)(curr); */

    /*     } else if (curr->mlength > nbytes) { /\* Split big block     *\/ */
    /*         leftover = (struct memblk *)((uint32) curr + */
    /*                 nbytes); */
    /*         prev->mnext = leftover; */
    /*         leftover->mnext = curr->mnext; */
    /*         leftover->mlength = curr->mlength - nbytes; */
    /*         prptr->prmemlist.mlength -= nbytes; */
    /*         restore(mask); */
    /*         return (char *)(curr); */
    /*     } else {            /\* Move to next block   *\/ */
    /*         prev = curr; */
    /*         curr = curr->mnext; */
    /*     } */
    /* } */
    /* restore(mask); */
    return (char *)SYSERR;
}
