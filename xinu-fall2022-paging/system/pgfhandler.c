/* pgfhandler.c - pgfhandler */

#include <xinu.h>

pgf_t pgferr; /* Error code */
uint32 pgfaddr; /* Faulty address */

// PGF Invariant:
// If a page is marked as allocated,
// then the page will always have
// a corresponding PDE and PTE
inline void __pde_inv_check() {
    pd_t *pde = getpde(pgfaddr);

    if(pde->pd_pres == 0) {
        log_pgf("- Page Table is absent \n");
        pt_t *ptptr = newpt(currpid);
        if(ptptr == (pt_t*) SYSERR) {
            panic("Cannot find a free frame in region D \n");
        }
        pde->pd_pres = 1;
        pde->pd_write = 1;
        pde->pd_base = ((uint32) ptptr / NBPG);
    }
}

void __debug_pgf() {
    struct procent *prptr = &proctab[currpid];

    log_pgf("----- PAGE FAULT ----- \n");
    log_pgf("- PID: %d \n", currpid);
    log_pgf("- Error code: 0x%x \n", pgferr);
    log_pgf("- Errorneous address: 0x%x \n", pgfaddr);
    log_pgf("- Allocated: %d \n", prptr->pralloc[VHNUM(pgfaddr)]);
    log_pgf("- Page number: %d \n", PGNUM(pgfaddr));
    log_pgf("- PDIDX: %d \n", PDIDX(pgfaddr));
    log_pgf("- PTIDX: %d \n", PTIDX(pgfaddr));
}


/*------------------------------------------------------------------------
 * pgfhandler - high level page fault interrupt handler
 *------------------------------------------------------------------------
 */
void pgfhandler(void) {
    struct procent *prptr = &proctab[currpid];

    __debug_pgf();

    // Check if error was caused due to access violation issue
    if(pgferr.pgf_pres == 1) {
        log_pgf("- System page fault PID %d \n", currpid);
        log_pgf("----- ---------- ----- \n");
        return;
    }

    // Check if page was not allocated by vmhgetmem
    if(prptr->pralloc[VHNUM(pgfaddr)] == 0) {
        kprintf("- Segmentation fault PID %d \n", currpid);
        log_pgf("----- ---------- ----- \n");
        kill(currpid);
        return; // For completeness's sake
    }

    // Page is allocated but not present in E1
    // There are multiple scenarios from here.
    // Before we move on to these scenarios,
    // Check for system invariants

    // Always ensure the existence of PDE and PTE!
    __pde_inv_check();

    // Get state of the system
    // To select which scenario we are in
    bool8 e1full, e2full, swapped;
    e1full  = !hasfreeframe(REGION_E1);
    e2full  = !hasfreeframe(REGION_E2);
    swapped = getpte(pgfaddr)->pt_swap;
    log_pgf("- E2Full=%d E1Full=%d Swapped=%d \n", e1full, e2full, swapped);

    // Scenarios
    if(swapped) { // Old frame is in E2
        if(e1full && e2full) {
            // Swap it into E1
            swapframe();
        } else if(e1full) {
            // Make space in E1 and restore
            evictframe();
            restoreframe();
        } else {
            // Restore it into E1
            restoreframe();
        }
    } else { // Fresh frame in E1 required
        while(e1full && e2full) {
            // Block until space in
            // E1 or E2
            log_pgf("- Blocking \n");
            log_pgf("----- ---------- ----- \n");
            frameblock();
            __debug_pgf();
            log_pgf("- Unblocked from frame block! \n");
            e1full = !hasfreeframe(REGION_E1);
            e2full = !hasfreeframe(REGION_E2);
        }

        if(e1full) {
            // Make space in E1 and assign new frame
            evictframe();
            mapfreeframe();
        } else {
            // Assign new frame directly
            mapfreeframe();
        }
    }

    if(!swapped) { // Was a new page
        // Increase reference count of PT frame by 1
        pd_t *pde = getpde(pgfaddr);
        invpt[INIDX(pde->pd_base)].fr_refcnt++;
    }

    log_pgf("----- ---------- ----- \n");
    return;
}
