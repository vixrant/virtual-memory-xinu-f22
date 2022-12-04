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
        pushal          # Save registers
        cli             # Disable further interrupts

        // Copy errorneous address
        movl    %cr2, %ebx
        movl    %ebx, pgfaddr
        // Copy error code
        movl    32(%esp), %ebx
        movl    %ebx, pgferr

        // Bottom function
        call    pgfhandler  # Call high level handler

        sti             # Restore interrupt status
        popal           # Restore registers
        popal           # Restore registers
        addl    $4, %esp # Discard error code
        iret            # Return from interrupt
