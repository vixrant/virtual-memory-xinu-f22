/* paginginit.c - paginginit */

#include <xinu.h>

int16 paginginit(pd_t *nullprocpd) {
    uint16 i; /* Initialization loop iterator */
    uint16 fi; /* Initialization loop iterator, for frame */
    pt_t *ptptr;

    // 1. Set all entries in inverted page table to FREE
    for(fi = 0; fi < NFRAMES; fi++) {
        invpt[fi].fr_state = FR_FREE;
    }

    // 2. Set up shared page tables
    for(i = 0; i < 5; i++) {
        // Create an newpt
        ptptr = newpt();
        if(ptptr == (pt_t*) SYSERR) {
            return SYSERR;
        }

        // Initialize each entry
        for(fi = 0; fi < NENTRIES; fi++) {
            // Presence bit
            ptptr[fi].pt_pres = 1;

            // Base bits
            if(i == 4) // Regions G
                ptptr[fi].pt_base = (NENTRIES * REGION_G_FRAME0) + fi; // page id + frame id
            else // Regions A - E2
                ptptr[fi].pt_base = (NENTRIES * i) + fi; // page id + frame id
        }

        // Set it to the global array for sharing
        identity_pt[i] = ptptr;
    }

    // 3. Set up null process's page directory
    for(i = 0; i < 5; i++) {
        // Presence bit
        nullprocpd[i].pd_pres = 1;

        // Base bits
        if(i == 4) // Regions G
            nullprocpd[REGION_G_FRAME0].pd_base = ((unsigned int) identity_pt[4]) >> 12;
        else // Regions A - E2
            nullprocpd[i].pd_base = ((unsigned int) identity_pt[i]) >> 12;
    }

    return OK;
}
