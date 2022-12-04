/* testlab6 */

#include <xinu.h>

//*DEBUG**********************************************************************/

static void debugpagetable(uint32 addr) {
    uint32 i;
    pd_t *pd = proctab[currpid].prpd;
    pt_t *pt = NULL;

    pdf("--- DEBUGGING PAGE TABLES --- \n");
    pdf("Address: 0x%08x \n", addr);
    pdf("Page Directory Index: %d \n", PDIDX(addr));
    pdf("Page Table Index: %d \n", PTIDX(addr));
    pdf("PDE: 0x%08x \n", *getpde(addr));
    pdf("PTE: 0x%08x \n", *getpte(addr));
    pdf("Frame: %d \n", getframenum(addr));
    pdf("Page directory crawl: \n");
    for(i=0 ; i<NENTRIES ; i++) {
        if(pd[i].pd_pres) {
            pdf("PDE %d present : 0x%08x \n", i, pd[i]);
        }

        if(i == PDIDX(addr)) {
            if(pd[i].pd_pres == 0) {
                pdf("PDE for addr absent \n");
            } else {
                pdf("PDE for addr present \n");
                pt = (pt_t*) (pd[i].pd_base << 12);
            }
        }
    }

    if(pt != NULL) {
        for(i=0 ; i<NENTRIES ; i++) {
            if(pt[i].pt_pres) {
                pdf("PTE %d present : 0x%08x \n", i, pt[i]);
            }
        }
    }
    pdf("----------------------------- \n");
}

//*TESTS**********************************************************************/

void test_mapfreeframe() {
    // Scenario: Free frames in E1
    pdf("----- TEST MAPFREEFRAME ----- \n");

    pdf("@ Requesting one page \n");
    char *x = vmhgetmem(1);

    pdf("@ Accessing all locations of 0x%08x \n", x);
    uint32 i;
    for(i = 0; i < NBPG; i++) {
        if(i < 10)
            x[i] = 'a';
        else
            x[i] = '\0';
    }

    pdf("@ x = %s \n", x);

    debugpagetable(x);

    pdf("@ Freeing frame \n");
    vmhfreemem(x, 1);
}

void test_evictframe() {
    // Scenario: E1 is full, Free frames in E2
    pdf("----- TEST EVICTFRAME ----- \n");
    char *x;
    uint32 i;

    pdf("@ Requesting one page \n");
    x = vmhgetmem(1);
    for(i = 0; i < NBPG; i++) {
        if(i < 10)  x[i] = 'a';
        else        x[i] = '\0';
    }
    pdf("@ x = %s \n", x);

    pdf("@ Setting all frames in E1 to occupied by null \n");
    for(i=0 ; i<NFRAMES_E1 ; i++) {
        fidx16 f = getfreeframe(REGION_E1);
        invtakeframe(f, NULLPROC, FR_PTEUNUSED);
    }

    pdf("@ Requesting one page \n");
    x = vmhgetmem(1);

    pdf("@ Accessing preserved page \n");
    x = 0x00be8000;
    pdf("@ x = %s \n", x);
}

void test_evictframe_restoreframe() {
    // Scenario: E2 has the frame that we want to restore
    pdf("----- TEST RESTOREFRAME ----- \n");

    char *x, *y;
    uint32 i;

    pdf("@ Requesting one page \n");
    x = vmhgetmem(1);
    for(i=0; i<NBPG; i++) {
        if(i < 10)  x[i] = 'a';
        else        x[i] = '\0';
    }
    pdf("@ x = %s \n", x);
    pdf("@ x is mapped to frame %d \n", getframenum(x));

    pdf("@ Setting all frames in E1 to occupied by null \n");
    for(i=0 ; i<NFRAMES_E1 ; i++) {
        fidx16 f = getfreeframe(REGION_E1);
        invtakeframe(f, NULLPROC, FR_PTEUNUSED);
    }

    pdf("@ Requesting one more page \n");
    y = vmhgetmem(1);

    pdf("@ Accessing preserved page \n");
    y = 0x00be8000;
    pdf("@ y = %s \n", y);

    pdf("@ Accessing old location again \n");
    for(i=0; i<10; i++) {
        x[i] = 'b';
    }
    pdf("@ x is mapped to frame %d \n", getframenum(x));
    pdf("@ printing 2025 frame: %s \n", (2025 << 12));
    pdf("@ x = %s \n", x);
}
