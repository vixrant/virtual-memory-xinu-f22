/* paginginit.c - paginginit */

#include <xinu.h>

static int16 _identity_pt_init(void) {
    uint16 i, j; /* Initialization loop iterator */
    pt_t *ptptr;

    for(i = 0; i < 5; i++) {
        // Create an newpt
        if((ptptr = newpt()) == (pt_t*) SYSERR) {
            return SYSERR;
        }

        // Initialize each entry
        for(j = 0; j < NENTRIES; j++) {
            ptptr[j].pt_pres = 1;

            // Base bits
            if(i == 4) // Regions G
                ptptr[j].pt_base = (NENTRIES * REGION_G_PD) + j; // directory id + table id
            else // Regions A - E2
                ptptr[j].pt_base = (NENTRIES * i) + j; // directory id + table id
        }

        // Set it to the global array for sharing
        identity_pt[i] = ptptr;
    }

    return OK;
}

int16 paginginit(pd_t *nullprocpd) {
    uint16 i; /* Initialization loop iterator */

    // 1. Set all entries in inverted page table to FREE
    for(i = 0; i < NFRAMES; i++) {
        invpt[i].fr_state = FR_FREE;
    }

    // 2. Set up shared page tables
    if(_identity_pt_init() == SYSERR) {
        return SYSERR;
    }

    // 3. Set up null process's page directory
    for(i = 0; i < 4; i++) { // Regions A - E2
        nullprocpd[i].pd_pres = 1;
        nullprocpd[i].pd_base = ((unsigned int) identity_pt[i]) / NBPG;
    }

    // Regions G
    nullprocpd[REGION_G_PD].pd_pres = 1;
    nullprocpd[REGION_G_PD].pd_base = ((unsigned int) identity_pt[4]) / NBPG;

    // 4. Set-up CR3 with null process's page directory
    /* pdsw(nullprocpd); */
    asm ("movl %0, %%cr3" : : "r" (nullprocpd));
    pdf("paginginit - set CR3 \n");

    // 5. Set page fault handler
	set_evec(14, (uint32) pgfdisp);

    // 5. Enable paging
    pagingenable();
    pdf("paginginit - enabled paging \n");

    return OK;
}
