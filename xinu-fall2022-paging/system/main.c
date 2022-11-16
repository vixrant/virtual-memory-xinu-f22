/*  main.c  - main */

#include <xinu.h>
#include <stdio.h>

process	main(void)
{

	/* Start the network */
	/* DO NOT REMOVE OR COMMENT THIS CALL */
	/* netstart(); */

	// Insert test code below
	kprintf("Hello World\n");

	int *x, *y;

	y = x = (int*) vmhgetmem(1);
	pdf("Got memory %d \n", x);

	x = (int*) vmhgetmem(1023);
	pdf("Got memory %d \n", x);

	x = (int*) vmhgetmem(1);
	pdf("Got %d on requesting more than 1024 pages \n", x);

	vmhfreemem((char*) y, 1);

	x = (int*) vmhgetmem(1);
	pdf("Got memory %d after freeing %d \n", x, y);

	return OK;
}
