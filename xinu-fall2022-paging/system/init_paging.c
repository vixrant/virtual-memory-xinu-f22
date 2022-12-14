/* init_paging.c - init_paging */

#include <xinu.h>

// Setup 5 page tables
// 4 of them are sequential for A - E2
// 1 of them is at 576 for G
static inline int16 __identity_pt_init(void) {
    uint16 i, j; /* Initialization loop iterator */
    pt_t *ptptr;

    for(i = 0; i < 5; i++) {
        // Create an newpt
        if((ptptr = newpt(NULLPROC)) == (pt_t*) SYSERR) {
            return SYSERR;
        }

        // Initialize each entry
        for(j = 0; j < NENTRIES; j++) {
            ptptr[j].pt_pres = 1;
            ptptr[j].pt_write = 1;

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

/*---------------------------------------------------------------------------
 *  init_paging - Initializes paging. Refer to each step in comments.
 *---------------------------------------------------------------------------
 */
void init_paging(void) {
    struct procent *prptr = &proctab[NULLPROC];

    // 1. Set-up frame management
    init_invpt();

    // 2. Set up shared page tables
    if(__identity_pt_init() == SYSERR) {
        panic("Could not set up shared page tables \n");
    }

    // 3. Set up null process's page directory
    prptr->prpd = newpd(NULLPROC);

    // 4. Set-up CR3 with null process's page directory
    log_init("paginginit - set CR3 \n");
    pdsw(prptr->prpd);

    // 5. Set page fault handler
    set_evec(14, (uint32) pgfdisp);

    // 6. Enable paging
    pagingenable();
    log_init("paginginit - enabled paging \n");
}
