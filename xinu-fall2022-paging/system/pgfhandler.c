/* pgfhandler.c - pgfhandler */

#include <xinu.h>

pgf_t pgferr; /* Error code */
uint32 pgfaddr; /* Faulty address */

/*------------------------------------------------------------------------
 * pgfhandler - high level page fault interrupt handler
 *------------------------------------------------------------------------
 */
void pgfhandler(void) {
    pdf("----- PAGE FAULT ----- \n");
    pdf("Errorneous address: %x \n", pgfaddr);

    uint32 pnum = pgfaddr / NBPG;
    pdf("Page number: %x \n", pnum);

    if(pgferr.pgf_pres) {
        pdf("Page as absent, allocate a frame \n");
    }

    panic("Page fault processing complete...\n");
}
