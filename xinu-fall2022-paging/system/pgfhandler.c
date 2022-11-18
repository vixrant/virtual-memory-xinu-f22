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
    log_pgf("PID: %d \n", currpid);
    log_pgf("Error code: 0x%x \n", pgferr);
    log_pgf("Errorneous address: 0x%x \n", pgfaddr);
    log_pgf("Allocated: %d \n", prptr->pralloc[VHNUM(pgfaddr)]);
    log_pgf("Page number: 0x%x \n", PGNUM(pgfaddr));
    log_pgf("PDIDX: 0x%x \n", PDIDX(pgfaddr));
    log_pgf("PTIDX: 0x%x \n", PTIDX(pgfaddr));

    // Check if error was caused due to page being absent
    if(pgferr.pgf_pres == 0) {
        // Check if page was not allocated in the page directory
        if(prptr->pralloc[VHNUM(pgfaddr)] == 0) {
            kprintf("Segmentation fault PID %d \n", currpid);
            kill(currpid);
            return; // For completeness's sake
        }

        // Page is allocated but ont present in E1
        // Allocate new frame in E1 and map it in PTE

        // Get page directory entry
        pd_t *pde = getpde((char*) pgfaddr);
        // Check if PDE is absent, if yes then allocate a new one
        if(pde->pd_pres == 0) {
            pde->pd_pres = 1;
            pde->pd_write = 1;
            pde->pd_base = ((uint32) newpt(currpid) / NBPG);
        }

        // Allocate a new frame
        fidx16 frame_num = getfreeframe(REGION_E1);
        if(frame_num == SYSERR) {
            // TODO: Block here
            panic("Cannot find a free frame in region E1 \n");
        }
        allocaframe(frame_num, currpid);

        // Get page table entry
        pt_t *pte = getpte((char*) pgfaddr);
        // Map page to frame
        pte->pt_pres = 1;
        pte->pt_write = 1;
        pte->pt_base = frame_num;

        log_pgf("----- ---------- ----- \n");
        return;
    }

    kprintf("No approach to handle this page fault: addr 0x%x code 0x%x \n", pgfaddr, pgferr);
    panic("Page fault handler panic \n");
}
