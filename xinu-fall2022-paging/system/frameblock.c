/* frameblock.c -  */

#include <xinu.h>

// Block queue
pid32  fbqueue[NPROC];
uint32 fbqhead = SYSERR;
uint32 fbqtail = SYSERR;

// Insert into block queue
inline syscall __insert_fbqueue(pid32 pid) {
    if(fbqhead == SYSERR) {
        // Queue empty
        fbqhead = fbqtail = 0;
    }

    uint16 pos = (fbqtail + 1) % NPROC;

    if(pos == fbqhead) {
        // Queue full
        return SYSERR;
    }

    fbqueue[pos] = pid;
    fbqtail = pos;

    return OK;
}

// Dequeue from block queue
inline pid32 __dequeue_fbqueue() {
    if(fbqhead == SYSERR) {
        // Queue empty
        return SYSERR;
    }

    pid32 retval = fbqueue[fbqhead];

    if(fbqhead == fbqtail) {
        // retval as the only element
        fbqhead = fbqtail = SYSERR;
    } else {
        fbqhead = (fbqhead + 1) % NPROC;
    }

    return retval;
}


/*------------------------------------------------------------------------
 *  frameblock  -  Blocks a process waiting on free frames in E1 or E2
 *------------------------------------------------------------------------
 */
syscall frameblock() {
    intmask mask;
    struct procent *prptr = &proctab[currpid];

    mask = disable();

    // Insert into wait queue
    if(__insert_fbqueue(currpid) == SYSERR) {
        restore(mask);
        return SYSERR;
    }

    prptr->prstate = PR_FRBLOCK;
    resched();
    restore(mask);
    return OK;
}


/*------------------------------------------------------------------------
 *  framewakeup  -  Wakes up processes waiting on free frames in E1 or E2
 *------------------------------------------------------------------------
 */
void framewakeup() {
    resched_cntl(DEFER_START);
    while(TRUE) {
        if(!hasfreeframe(REGION_E1) && !hasfreeframe(REGION_E2)) {
            break;
        }

        pid32 pid = __dequeue_fbqueue();
        if(pid == SYSERR) {
            break;
        }

        ready(pid);
    }
    resched_cntl(DEFER_STOP);
}
