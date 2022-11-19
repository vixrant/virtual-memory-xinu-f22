/* pagingidx.c - getpde, getpte */

#include <xinu.h>

pd_t *getpde(char* addr) {
    pd_t *pdptr = proctab[currpid].prpd;
    pd_t *pde = &pdptr[PDIDX((uint32) addr)];
    return pde;
}

pt_t *getpte(char* addr) {
    pd_t *pde = getpde(addr);
    pt_t *ptptr = (pt_t*) (pde->pd_base * NBPG);
    pt_t *pte = &ptptr[PTIDX((uint32) addr)];
    return pte;
}

fidx16 getframenum(char* addr) {
    pt_t *pte = getpte(addr);
    fidx16 frame_num = pte->pt_base;
    return frame_num;
}
