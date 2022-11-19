/* pgfhandler.c - pgfhandler */

#include <xinu.h>

pgf_t pgferr; /* Error code */
uint32 pgfaddr; /* Faulty address */

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

    // If error was caused due to access violation issue
    if(pgferr.pgf_pres == 1) {
        log_pgf("System page fault");
        log_pgf("----- ---------- ----- \n");
        return;
    }

    log_pgf("- Maping was absent \n");

    // Check if page was not allocated in the page directory
    if(prptr->pralloc[VHNUM(pgfaddr)] == 0) {
        kprintf("- Segmentation fault PID %d \n", currpid);
        kill(currpid);
        return; // For completeness's sake
    }

    // Page is allocated but not present in E1
    // Allocate new frame in E1 and map it in PTE

    // 1. Check if PDE is absent, if so then allocate a new one
    pd_t *pde = getpde((char*) pgfaddr);
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
    log_pgf("- PDE 0x%08x \n", *pde);

    // 2. Get a new frame
    log_pgf("- Getting a frame in E1 \n");
    fidx16 frame_idx = getfreeframe(REGION_E1);
    if(frame_idx == SYSERR) {
        // TODO: Block here
        panic("Cannot find a free frame in region E1 \n");
    }
    allocaframe(frame_idx, currpid);

    // 3. Map page to frame
    pt_t *pte = getpte((char*) pgfaddr);
    log_pgf("- Mapping %d -> %d \n", PGNUM(pgfaddr), frame_idx + FRAME0);
    pte->pt_pres = 1;
    pte->pt_write = 1;
    pte->pt_base = frame_idx + FRAME0;
    log_pgf("- PTE after updating 0x%08x \n", *pte);

    log_pgf("----- ---------- ----- \n");
    return;
}
