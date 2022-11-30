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

// Assigns new frame in E1 to page
inline void __assign_new_frame() {
    fidx16 frame_num = SYSERR;

    // 1. Get a new frame
    log_pgf("- Assigning to a new frame in E1 \n");
    frame_num = getfreeframe(REGION_E1);
    while(frame_num == SYSERR) {
        // Could not get a frame in E1
        // Try to evict a frame to E2
        while(evictframe() == SYSERR) {
            // No space in E2
            // Block until frames become free
            panic("TODO: Block");
        }

        // Try to get a frame again
        frame_num = getfreeframe(REGION_E1);
    }

    // 2. Occupy frame
    allocaframe(frame_num, currpid, PGNUM(pgfaddr));

    // 3. Map page to frame
    pt_t *pte = getpte(pgfaddr);
    log_pgf("- Mapping %d -> %d \n", PGNUM(pgfaddr), frame_num);
    pte->pt_pres = 1;
    pte->pt_write = 1;
    pte->pt_swap = 0;
    pte->pt_base = frame_num;
    log_pgf("- PTE after updating 0x%08x \n", *pte);
}

// Restores the backed up frame in E2 to E1
inline void __restore_swapped_frame() {
}

/*------------------------------------------------------------------------
 * pgfhandler - high level page fault interrupt handler
 *------------------------------------------------------------------------
 */
void pgfhandler(void) {
    struct procent *prptr = &proctab[currpid];

    log_pgf("----- PAGE FAULT ----- \n");
    log_pgf("- PID: %d \n", currpid);
    log_pgf("- Error code: 0x%x \n", pgferr);
    log_pgf("- Errorneous address: 0x%x \n", pgfaddr);
    log_pgf("- Allocated: %d \n", prptr->pralloc[VHNUM(pgfaddr)]);
    log_pgf("- Page number: 0x%x \n", PGNUM(pgfaddr));
    log_pgf("- PDIDX: 0x%x \n", PDIDX(pgfaddr));
    log_pgf("- PTIDX: 0x%x \n", PTIDX(pgfaddr));

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
    // 1. Page was never assigned to a frame in E1
    // 2. Page was assigned a frame, but swapped out to E2
    // Before we move on to these scenarios,
    // Check for system invariants

    // Always ensure the existence of PDE and PTE!
    __pde_inv_check();

    // If a page was never assigned a frame,
    // it's pt_swap bit in page table will be 0
    pt_t *pte = getpte(pgfaddr);
    if(pte->pt_swap == 0) {
        log_pgf("- Page was never assigned \n");
        __assign_new_frame();
    } else {
        log_pgf("- Page was swapped out \n");
        panic("Not implemented \n");
    }


    log_pgf("----- ---------- ----- \n");
    return;
}
