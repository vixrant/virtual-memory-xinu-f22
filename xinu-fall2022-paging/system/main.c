/*  main.c  - main */

#include <xinu.h>

//*DEBUG**********************************************************************/
#if XINUDEBUG

void debugpagetable(uint32 addr) {
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
				pt = (pd[i].pd_base << 12);
			}
		}
	}

	if(pt) {
		for(i=0 ; i<NENTRIES ; i++) {
			if(pt[i].pt_pres) {
				pdf("PTE %d present : 0x%08x \n", i, pt[i]);
			}
		}
	}
	pdf("----------------------------- \n");
}

#endif

//*TEST***********************************************************************/
#if XINUTEST

void testframemgmt(void) {
	uint16 i;

	pdf("@ Allocating %d pages \n", NFRAMES_E1);
	for(i=0 ; i<NFRAMES_E1; i++) {
		pdf("@ i=%d \n", i);
		allocaframe(getfreeframe(REGION_E1), currpid);
	}

	pdf("@ Requesting one more \n");
	getfreeframe(REGION_E1);

	pdf("@ Deallocating frame 2 and 3 \n");
	deallocaframe(2050);
	deallocaframe(3000);

	pdf("@ Requesting one more \n");
	allocaframe(getfreeframe(REGION_E1), currpid);
	allocaframe(getfreeframe(REGION_E1), currpid);
}

void testgetmem1(void) {
	pdf("@ Requesting one page \n");
	char *x = vmhgetmem(1);
	pdf("@ Accessing all locations of this page \n");
	uint32 i;
	for(i = 0; i < NBPG; i++) {
		if(i < 10)
			x[i] = 'a';
		else
			x[i] = '\0';
	}
	pdf("@ x = %s \n", x);
	pdf("@ Printing debug information \n ");
	debugpagetable(x);
}

// Free memory

void testfreemem1(void) {
	int *x, *y;

	pdf("@ Requesting 2 pages \n");
	x = (int*) vmhgetmem(2);
	pdf("@ Got memory 0x%08x requesting 1 \n", x);

	pdf("@ Requesting 1022 pages \n");
	x = (int*) vmhgetmem(1022);
	y = x;
	pdf("@ Got memory 0x%08x requesting 1022 \n", x);

	pdf("@ Requesting more than 1024 page \n");
	x = (int*) vmhgetmem(1);
	pdf("@ Got %d \n", x);

	pdf("@ Freeing memory  %08x \n", y);
	vmhfreemem((char*) y, 1022);

	pdf("@ Requesting 1 pages \n");
	x = (int*) vmhgetmem(1);
	pdf("@ Got memory %08x on requesting 1 \n", x, y);

	pdf("@ Requesting 1 pages \n");
	x = (int*) vmhgetmem(1);
	pdf("@ Got memory %08x on requesting 1 \n", x, y);
}

void testfreemem2(void) {
	int *x, *y;

	pdf("@ Requesting 2 pages \n");
	x = (int*) vmhgetmem(1);
	y = (int*) vmhgetmem(1);

	*x = 69;
	*y = 96;
	pdf("@ x = %d y = %d \n", *x, *y);

	pdf("@ Freeing 2 pages then accessing \n");
	vmhfreemem(x, 1);
	vmhfreemem(y, 1);

	pdf("@ Accessing locations \n");
	pdf("@ x = %d \n", *x);
	pdf("@ y = %d \n", *y);
}

// Two process test

int *testsharedlocation;
int sem;

void testtwoproc11(void) {
	pdf("@1 Requesting 1 location \n");
	testsharedlocation = (int*) vmhgetmem(1);

	pdf("@1 Setting 0x%x = 69 \n", testsharedlocation);
	*testsharedlocation = 69;

	pdf("@1 Allowing process 2 to run \n");
	sem = 0;

	while(sem == 0) ;
}

void testtwoproc12(void) {
	while(sem == 1) ;

	int t = 0;

	pdf("@2 Process unblocked, trying to access location \n");
	t = *testsharedlocation;

	pdf("@2 ERROR Was able to access t = %d \n", t);
}

void testtwoproc1(void) {
	sem = 1;
	resume(create(testtwoproc11, INITSTK, INITPRIO, "Process 1", 0));
	resume(create(testtwoproc12, INITSTK, INITPRIO, "Process 2", 0));
}

#endif
//*MAIN***********************************************************************/

process	main(void)
{
	#if XINUTEST

	testfreemem2();

	#endif

	return OK;
}
