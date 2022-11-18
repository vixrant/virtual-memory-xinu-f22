/*  main.c  - main */

#include <xinu.h>
#include <stdio.h>

void debugpagetable(uint32 addr) {
	uint32 i;
	pd_t *pd = proctab[currpid].prpd;

	pdf("--- DEBUGGING PAGE TABLES --- \n");
	pdf("Address: 0x%x \n", addr);
	pdf("Page Directory Index: 0x%x \n", PDIDX(addr));
	pdf("Page Table Index: 0x%x \n", PTIDX(addr));
	pdf("Page directory crawl: \n");
	for(i=0 ; i<NENTRIES ; i++) {
		if(pd[i].pd_pres) {
			pdf("PDE %i present : 0x%x \n", pd[i]);
		}

		if(i == PDIDX(addr) && pd[i].pd_pres == 0) {
			pdf("PDE for addr absent \n");
		}
	}
	pdf("----------------------------- \n");
}

process	main(void)
{

	/* Start the network */
	/* DO NOT REMOVE OR COMMENT THIS CALL */
	/* netstart(); */

	// Insert test code below
	kprintf("Hello World\n");

	int *x, *y;

	y = x = (int*) vmhgetmem(1);
	pdf("Got memory 0x%x after requesting 1 \n", x);

	x = (int*) vmhgetmem(1023);
	pdf("Got memory 0x%x after requesting 1023 \n", x);

	x = (int*) vmhgetmem(1);
	pdf("Got %d on requesting more than 1024 pages \n", x);

	vmhfreemem((char*) y, 1);
	x = (int*) vmhgetmem(1);
	pdf("Got memory %x after freeing %x \n", x, y);

	return OK;
}
