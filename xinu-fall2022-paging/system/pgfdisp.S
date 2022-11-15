/* pgfdisp.s - pgddisp */

/*------------------------------------------------------------------------
 * pgfdisp  -  Interrupt dispatcher for page fault interrupts
 *------------------------------------------------------------------------
 */
#include <icu.s>
        .text
        .globl  pgfdisp         # Page fault interrupt dispatcher
pgfdisp:
        pushal          # Save registers
        cli             # Disable further interrupts
        movb    $EOI,%al    # Reset interrupt
        outb    %al,$OCW1_2

        movl    %cr2, %ebx
        pushl   %ebx // Push CR2
        pushl   36(%esp) // Push Error Code
        call    pgfhandler  # Call high level handler
        popl    %ebx
        popl    %ebx

        sti             # Restore interrupt status
        popal           # Restore registers
        iret            # Return from interrupt
