.section ".text.boot"

.global _start

_start:
    // read cpu id
    mrs x0, mpidr_el1
    
    // only use core0, the other core goes into infinite loop
    and x0, x0, #3
    cbz x0, 2f

1:  
    wfe
    b       1b                     

2:
    bl     from_el2_to_el1             // switch to EL1 
    bl     set_exception_vector_table  // set up exception vector table 
    
    ldr    x0, =_start                  
    mov    sp, x0
    ldr    x0, =__bss_start             
    ldr    x1, =__bss_end
    sub    x1, x1, x0

memzero:
    cbz    x1, movesp
	str    xzr, [x0], #8
	subs   x1, x1, #8
	cbnz   x1, memzero

movesp:
    mov    sp, #0x80000
    bl     main                    


set_exception_vector_table:
    adr x0, exception_vector_table  
    msr vbar_el1, x0
    ret

from_el2_to_el1:
    mov x0, (1 << 31) // EL1 uses aarch64
    msr hcr_el2, x0
    mov x0, 0x3c5 // EL1h (SPSel = 1) with interrupt disabled
    msr spsr_el2, x0
    msr elr_el2, lr
    eret // return to EL1

    