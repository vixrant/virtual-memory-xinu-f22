/* pagingenable.S - pagingenable */

    .text
    .globl  pagingenable

/*------------------------------------------------------------------------
 * pagingenable -  X86 paging enable by setting PG=1 in CR0
 *------------------------------------------------------------------------
 */
pagingenable:
    cli
    movl %cr0, %eax
    orl  $0x80000000, %eax
    movl %eax, %cr0
    sti
    ret
