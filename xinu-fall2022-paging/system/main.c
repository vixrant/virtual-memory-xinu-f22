/*  main.c  - main */

#include <xinu.h>

extern void test_mapfreeframe();
extern void test_evictframe();
extern void test_evictframe_restoreframe();
extern void test_swapframes();
extern void test_frameblock();

extern void test_lab6();

//*MAIN***********************************************************************/

process	main(void)
{
	#if XINUTEST
	/* resume(create(testframemgmt, INITSTK, INITPRIO + 10, "Test", 0)); */
	/* resume(create(testgetmem1,   INITSTK, INITPRIO + 10, "Test", 0)); */
	/* resume(create(testfreemem1,  INITSTK, INITPRIO + 10, "Test", 0)); */
	/* resume(create(testfreemem2,  INITSTK, INITPRIO + 10, "Test", 0)); */
	/* resume(create(testtwoproc1,  INITSTK, INITPRIO + 10, "Test", 0)); */

	/* resume(create(test_mapfreeframe, INITSTK, INITPRIO + 10, "Test", 0)); */
	// resume(create(test_evictframe, INITSTK, INITPRIO + 10, "Test", 0));
	// resume(create(test_evictframe_restoreframe, INITSTK, INITPRIO + 10, "Test", 0));
	// resume(create(test_swapframes, INITSTK, INITPRIO + 10, "Test", 0));
	// resume(create(test_frameblock, INITSTK, INITPRIO + 10, "Test", 0));
	resume(create(test_lab6, INITSTK, INITPRIO + 10, "Test", 0));
	#endif

	return OK;
}
