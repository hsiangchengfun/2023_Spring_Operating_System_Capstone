#include "mem.h"
#include "cpio.h"
#include "uart.h"
#include "utils.h"
#include "devtree.h"

void *DEVICETREE_CPIO_BASE = 0;

void cpio_ls(void) {
    struct cpio_newc_header *head =  (void*) DEVICETREE_CPIO_BASE;//(struct cpio_newc_header *) DTB_CPIO_BASE;//CPIO_BASE_RPI; //(void*)CPIO_QEMU_BASE;//
    uint32_t h_size = sizeof(struct cpio_newc_header);

    while (1) {
        char *f_name = ((void*) head) + h_size;

        

        if (!strncmp(f_name, "TRAILER!!!",10))break;

        mini_uart_puts(f_name);
        mini_uart_puts("\r\n");

        uint32_t n_size = atoi(head->c_namesize, 8);
        uint32_t f_size = atoi(head->c_filesize, 8);
        uint32_t offset   = h_size + n_size;
        if(offset % 4 != 0) offset = 4 * ((offset + 4) / 4);
        if(f_size % 4 != 0) f_size = 4 * ((f_size + 4) / 4);

        head = ((void*) head) + offset + f_size;
    }
}

void cpio_cat(void) {
    uint32_t h_size = sizeof(struct cpio_newc_header);
    struct cpio_newc_header *head =  (void*) DEVICETREE_CPIO_BASE;//(struct cpio_newc_header *) DTB_CPIO_BASE;//CPIO_BASE_QEMU; //CPIO_BASE_RPI;//(void*)CPIO_QEMU_BASE;//

    char input[BUFSIZE];

    mini_uart_puts("Filename: ");
    mini_uart_gets(input, BUFSIZE);
    

    while (1) {
        char *f_name = ((void*) head) + h_size;

        if (strncmp(f_name, CPIO_FOOTER, 10) == 0) {
            mini_uart_puts("No matching file\r\n"); 
            break;
        }

        uint32_t n_size = atoi(head->c_namesize, 8);
        uint32_t f_size = atoi(head->c_filesize, 8);
        uint32_t offset   = h_size + n_size;

        if(offset % 4 != 0) offset = 4 * ((offset + 4) / 4);

        if (strncmp(input, f_name, n_size) == 0) {
            char *content = ((void*) head) + offset;
            

            for (int i = 0; i < f_size; i++) {
                if (content[i] == '\n') {
                    mini_uart_puts("\r\n");
                } else {
                    mini_uart_putc(content[i]);
                }
            }

            if (content[f_size - 1] != '\n') {
                mini_uart_puts("\r\n");
            }
            break;
        }

        if(f_size % 4 != 0) f_size = 4 * ((f_size + 4) / 4);
        head  = ((void*) head) + offset + f_size;
    }


}

void cpio_exe(void) {
    uint32_t h_size = sizeof(struct cpio_newc_header);
    struct cpio_newc_header *head = (void*) DEVICETREE_CPIO_BASE;//(struct cpio_newc_header *) DTB_CPIO_BASE;//CPIO_BASE_QEMU; //CPIO_BASE_RPI;//

    char input[BUFSIZE];

    mini_uart_puts("Filename: ");
    mini_uart_gets(input, BUFSIZE);
    
    int no_match=1;

    while (1) {
        char *f_name = ((void*) head) + h_size;
        
        if (strncmp(f_name, CPIO_FOOTER, 10) == 0) {
            mini_uart_puts("No matching file\r\n"); 
            break;
        }
        

        uint32_t n_size = atoi(head->c_namesize, 8);
        uint32_t f_size = atoi(head->c_filesize, 8);
        uint32_t offset   = h_size + n_size;

        if(offset % 4 != 0) offset = 4 * ((offset + 4) / 4);

        if (strncmp(input, f_name, n_size) == 0) {
            char *prog_start = ((void*) head) + offset;
            char *sp = prog_start + STACKSIZE;
            //from el1 to el0
            
            // set spsr_el1 to 0x3c0 
            // and elr_el1 to the program’s start address.
            // elr_el1 -> program start addr
            asm volatile( "msr     elr_el1, %0" :: "r" (prog_start) );
            asm volatile( "mov     x20 ,  0x3c0" );
            asm volatile( "msr     spsr_el1, x20" );
            // set the user program’s stack pointer 
            // to a proper position by setting sp_el0.
            // sp_el0 -> usr prog stack addr
            asm volatile( "msr     sp_el0,  %0" :: "r" (sp) );
            asm volatile( "eret    ");
            break;
        }

        if(f_size % 4 != 0) f_size = 4 * ((f_size + 4) / 4);
        head  = ((void*) head) + offset + f_size;
    }

    if(no_match)mini_uart_puts("No matching file\r\n");

}


void initramfs_callback(char *n_name, char *p_name, struct fdt_prop* prop) {
    if (!strncmp(n_name, "chosen", 7) && !strncmp(p_name, "linux,initrd-start", 19)) {
        DEVICETREE_CPIO_BASE = (void*)((long unsigned int) fdt32_to_cpu(*((unsigned int*)(prop + 1))));
    }
}










