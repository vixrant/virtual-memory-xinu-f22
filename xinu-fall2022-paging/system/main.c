/*  main.c  - main */

#include <xinu.h>

//*DEBUG**********************************************************************/

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

//*TEST***********************************************************************/

void testgetmem1(void) {
	kprintf("@ Requesting one page \n");
	char *x = vmhgetmem(1);
	kprintf("@ Accessing all locations of this page \n");
	uint32 i;
	for(i = 0; i < NBPG; i++) {
		if(i < 10)
			x[i] = 'a';
		else
			x[i] = '\0';
	}
	kprintf("@ x = %s \n", x);
	kprintf("@ Printing debug information \n ");
	debugpagetable(x);
}

void testframemanagement() {
}

//*MAIN***********************************************************************/

process	main(void)
{
	/* Start the network */
	/* DO NOT REMOVE OR COMMENT THIS CALL */
	/* netstart(); */

	/* int *x, *y; */

	/* y = x = (int*) vmhgetmem(1); */
	/* pdf("Got memory 0x%08x after requesting 1 \n", x); */

	/* x = (int*) vmhgetmem(1023); */
	/* pdf("Got memory 0x%08x after requesting 1023 \n", x); */

	/* x = (int*) vmhgetmem(1); */
	/* pdf("Got %d on requesting more than 1024 pages \n", x); */

	/* vmhfreemem((char*) y, 1); */
	/* x = (int*) vmhgetmem(1); */
	/* pdf("Got memory %08x after freeing %08x \n", x, y); */

	testgetmem1();

	return OK;
}
