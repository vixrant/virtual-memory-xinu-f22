/* pgfhandler.c - pgfhandler */

#include <xinu.h>

/*------------------------------------------------------------------------
 * pgfhandler - high level page fault interrupt handler
 *------------------------------------------------------------------------
 */
interrupt pgfhandler(pgf_t err, uint32 addr) {
    pdf("----- PAGE FAULT ----- \n");
    pdf("Errorneous address: %x \n", addr);

    if(err.pgf_pres) {
        pdf("Present! \n");
    } else {
        pdf("Absent! \n");
    }

    if(err.pgf_write) {
        pdf("Write! \n");
    } else {
        pdf("Read! \n");
    }

    if(err.pgf_user) {
        pdf("User mode! \n");
    } else {
        pdf("Kernel mode! \n");
    }

    if(err.pgf_rsvd) {
        pdf("Reserved bit violation! \n");
    }

    if(err.pgf_isd) {
        pdf("Instruction fetch! \n");
    }

    if(err.pgf_pk) {
        pdf("Protection key violation! \n");
    }

    if(err.pgf_ss) {
        pdf("Shadow stack access! \n");
    }

    if(err.pgf_hlat) {
        pdf("HLAT paging! \n");
    }

    if(err.pgf_sgx) {
        pdf("SGX! \n");
    }

	panic("Page fault processing complete...\n");
}
