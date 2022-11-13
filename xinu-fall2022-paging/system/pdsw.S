/* pdsw.S - pdsw */

    .text
    .globl  pdsw

/*------------------------------------------------------------------------
 * pdsw -  X86 page directory switch, i.e. CR3 loading
 *------------------------------------------------------------------------
 */
pdsw:
    movl 4(%esp), %ecx
    movl %ecx, %cr3
    ret
