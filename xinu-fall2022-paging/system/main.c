/*  main.c  - main */

#include <xinu.h>

extern void testframemgmt();
extern void testgetmem1();
extern void testfreemem1();
extern void testfreemem2();
extern void testtwoproc1();

extern void test_mapfreeframe();
extern void test_evictframe();

//*MAIN***********************************************************************/

process	main(void)
{
	#if XINUTEST
	/* resume(create(testframemgmt, INITSTK, INITPRIO + 10, "Test", 0)); */
	/* resume(create(testgetmem1,   INITSTK, INITPRIO + 10, "Test", 0)); */
	/* resume(create(testfreemem1,  INITSTK, INITPRIO + 10, "Test", 0)); */
	/* resume(create(testfreemem2,  INITSTK, INITPRIO + 10, "Test", 0)); */
	/* resume(create(testtwoproc1,  INITSTK, INITPRIO + 10, "Test", 0)); */

	/* resume(create(test_mapfreeframe,  INITSTK, INITPRIO + 10, "Test", 0)); */
	resume(create(test_evictframe,  INITSTK, INITPRIO + 10, "Test", 0));
	#endif

	return OK;
}
