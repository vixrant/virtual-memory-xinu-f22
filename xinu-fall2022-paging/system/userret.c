/* userret.c - userret */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  userret  -  Called when a process returns from the top-level function
 *------------------------------------------------------------------------
 */
void	userret(void)
{
	pdf("userret - pid %d \n", getpid());
	kill(getpid());			/* Force process to exit */
}
