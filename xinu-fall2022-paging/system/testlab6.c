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
    // Scenario: E2 has the frame that we want to restore, but E1 is full
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

void test_swapframes() {
    // Scenario: E1 and E2 are full, Frame backed in E2
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
    while(hasfreeframe(REGION_E1)) {
        fidx16 f = getfreeframe(REGION_E1);
        invtakeframe(f, NULLPROC, FR_PTEUNUSED);
    }

    pdf("@ Requesting one more page \n");
    y = vmhgetmem(1);

    pdf("@ Accessing preserved page \n");
    y = 0x00be8000;
    pdf("@ y = %s \n", y);

    pdf("@ Setting all frames in E2 to occupied by null \n");
    while(hasfreeframe(REGION_E2)) {
        fidx16 f = getfreeframe(REGION_E2);
        invtakeframe(f, NULLPROC, FR_PTEUNUSED);
    }

    pdf("@ Accessing old location again \n");
    pdf("@ x = %s \n", x);
    for(i=0; i<10; i++) {
        x[i] = 'b';
    }
    pdf("@ x is mapped to frame %d \n", getframenum(x));
    pdf("@ printing 2025 frame: %s \n", (2025 << 12));
    pdf("@ x = %s \n", x);
}

///////////////////////////////////////////////////////////////////////////////

void test_frameblock_child() {
    // Scenario: E1 and E2 are full
    pdf("@ Child process started \n");

    pdf("@ Requesting one page, should block \n");
    char *x = vmhgetmem(1);

    pdf("@ Child with pid %d got page \n", getpid());
}

void test_frameblock() {
    pdf("----- TEST FRAMEBLOCK ----- \n");

    pdf("@ Requesting one page \n");
    char *x;
    x = vmhgetmem(1);

    pdf("@ Setting all frames in E1 to occupied by null \n");
    while(hasfreeframe(REGION_E1)) {
        fidx16 f = getfreeframe(REGION_E1);
        invtakeframe(f, NULLPROC, FR_PTEUNUSED);
    }

    pdf("@ Setting all frames in E2 to occupied by null \n");
    while(hasfreeframe(REGION_E2)) {
        fidx16 f = getfreeframe(REGION_E2);
        invtakeframe(f, NULLPROC, FR_PTEUNUSED);
    }

    pdf("@ Creating child processes \n");
	resume(create(test_frameblock_child, INITSTK, INITPRIO + 20, "Test child", 0));
	resume(create(test_frameblock_child, INITSTK, INITPRIO + 20, "Test child", 0));

    pdf("@ Parent frees frame \n");
    vmhfreemem(x, 1);
}

///////////////////////////////////////////////////////////////////////////////

void test_child() {
    pdf("@C Child process started \n");

    pdf("@C Requesting one page, this should block \n");
    char *x = vmhgetmem(1);

    pdf("@C Child with pid %d got page \n", getpid());
}

void test_lab6(void) {
    uint16 i;

    pdf("@P Parent process started \n");

    // ----- SCENARIO 1 -----
    pdf("$ CASE 1. E1 is free, new page \n");
    pdf("$ Here 2024th and 2025th frame should be assigned with mapnewframe \n");
    pdf("@P Requesting one page \n");
    char *x = vmhgetmem(1);
    for(i=0; i<NBPG; i++) {
        if(i < 10)  x[i] = 'a';
        else        x[i] = '\0';
    }
    pdf("@P x = %s \n", x);

    pdf("@P Requesting one more page \n");
    char *y = vmhgetmem(1);
    for(i=0; i<NBPG; i++) {
        if(i < 10)  y[i] = 'b';
        else        y[i] = '\0';
    }
    pdf("@P y = %s \n", y);

    // ----- SCENARIO 2 -----
    pdf("$ CASE 2. E1 is full, new page \n");
    pdf("$ The 2024th frame should be evicted and re-assigned \n");

    pdf("@P Setting all frames in E1 to occupied by null \n");
    while(hasfreeframe(REGION_E1)) {
        fidx16 f = getfreeframe(REGION_E1);
        invtakeframe(f, NULLPROC, FR_PTEUNUSED);
    }

    pdf("@P Requesting one more page, should cause eviction \n");
    char *z = vmhgetmem(1);
    for(i=0; i<NBPG; i++) {
        if(i < 10)  z[i] = 'c';
        else        z[i] = '\0';
    }
    pdf("@P z = %s \n", z);

    // ----- SCENARIO 3 -----
    pdf("$ CASE 3. E1 is full, E2 is full, old page \n");
    pdf("$ The frame with x should be swapped to 2025 \n");

    pdf("@P Setting all frames in E2 to occupied by null \n");
    while(hasfreeframe(REGION_E2)) {
        fidx16 f = getfreeframe(REGION_E2);
        invtakeframe(f, NULLPROC, FR_PTEUNUSED);
    }

    pdf("@P Printing all allocated pages \n");
    pdf("@P x = %s \n", x);
    pdf("@P y = %s \n", y);
    pdf("@P z = %s \n", z);

    // ----- SCENARIO 4 -----
    pdf("$ CASE 4. Frame block \n");
    pdf("$ The child processes should be blocked \n");

    resume(create(test_child, INITSTK, INITPRIO + 20, "Test child", 0));

    pdf("@P Parent frees frame \n");
    vmhfreemem(x, 1);
    vmhfreemem(y, 1);
    vmhfreemem(z, 1);

    // Parent blocks
    while(1) {
        ;
    }
}
