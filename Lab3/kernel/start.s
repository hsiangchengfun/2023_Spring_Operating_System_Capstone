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

3:
    cbz    x1, 4f                  
    str    xzr, [x0], #8
    sub    x1, x1, #8
    cbnz   x1, 3b

4:
    bl     main
    b      1b                      
