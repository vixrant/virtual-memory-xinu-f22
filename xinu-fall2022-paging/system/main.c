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

	pdf("@ Deallocating frame 2 \n");
	deallocaframe(2);

	pdf("@ Requesting one more \n");
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

void testfreemem1(void) {
	int *x, *y;

	pdf("@ Requesting 2 pages \n");
	y = x = (int*) vmhgetmem(2);
	pdf("@ Got memory 0x%08x requesting 1 \n", x);

	pdf("@ Requesting 1022 pages \n");
	x = (int*) vmhgetmem(1022);
	pdf("@ Got memory 0x%08x requesting 1022 \n", x);

	pdf("@ Requesting more than 1024 page \n");
	x = (int*) vmhgetmem(1);
	pdf("@ Got %d \n", x);

	pdf("@ Freeing memory  %08x \n", y);
	vmhfreemem((char*) y, 2);

	x = (int*) vmhgetmem(1);
	pdf("@ Got memory %08x on requesting 1 \n", x, y);
}

#endif
//*MAIN***********************************************************************/

process	main(void)
{
	testframemgmt();

	return OK;
}
