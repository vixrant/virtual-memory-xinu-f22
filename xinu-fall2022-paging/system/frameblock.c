/* frameblock.c -  */

#include <xinu.h>

// Block queue
pid32  fbqueue[NPROC];
uint32 fbqhead = SYSERR;
uint32 fbqtail = SYSERR;

// Insert into block queue in circular manner
inline syscall __insert_fbqueue(pid32 pid) {
    if(fbqhead == SYSERR) {
        fbqhead = 0;
        fbqtail = 0;
        fbqueue[fbqhead] = pid;
    } else {
        fbqtail = (fbqtail + 1) % NPROC;
        fbqueue[fbqtail] = pid;
    }
    return OK;
}

// Dequeue from block queue in circular manner
inline pid32 __dequeue_fbqueue() {
    pid32 pid = SYSERR;
    if(fbqhead != SYSERR) {
        pid = fbqueue[fbqhead];
        if(fbqhead == fbqtail) {
            fbqhead = SYSERR;
            fbqtail = SYSERR;
        } else {
            fbqhead = (fbqhead + 1) % NPROC;
        }
    }
    return pid;
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

    prptr->prstate = PR_FRAME;
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
