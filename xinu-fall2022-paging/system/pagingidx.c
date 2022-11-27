/* pagingidx.c - getpde, getpte, getframenum */

#include <xinu.h>

/*------------------------------------------------------------------------
 * getpde - Get PDE from current process's PD
 *------------------------------------------------------------------------
 */
pd_t *getpde(uint32 addr) {
    pd_t *pdptr = proctab[currpid].prpd;
    pd_t *pde = &pdptr[PDIDX((uint32) addr)];
    return pde;
}

/*------------------------------------------------------------------------
 * getpte - Get PTE from current process's PD
 *------------------------------------------------------------------------
 */
pt_t *getpte(uint32 addr) {
    pd_t *pde = getpde(addr);
    pt_t *ptptr = (pt_t*) (pde->pd_base * NBPG);
    pt_t *pte = &ptptr[PTIDX((uint32) addr)];
    return pte;
}

/*------------------------------------------------------------------------
 * getframenum - Get frame for addr from current process's PD
 *------------------------------------------------------------------------
 */
fidx16 getframenum(char* addr) {
    pt_t *pte = getpte(addr);
    fidx16 frame_num = pte->pt_base;
    return frame_num;
}
