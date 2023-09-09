#include "devtree.h"
#include "sched.h"
#include "fork.h"
#include "string.h"
#include "memory.h"
#include "cpio.h"
#include "mini_uart.h"
#include "utils.h"


void *DEVTREE_CPIO_BASE = 0;
void *DEVTREE_CPIO_END = 0;

#define BUFSIZE 256

uint32_t atoi(char *s, uint32_t len) {

    uint32_t n = 0;

    for (int i=0; i<len; i++) {
        n *= 16;
        if (s[i] >= '0' && s[i] <= '9') {
            n += s[i] - '0';
        } else if (s[i] >= 'A' && s[i] <= 'F') {
            n += s[i] - 'A' + 10;
        }
    }

    return n;

}

void initramfs_callback(char *n_name, char *p_name, struct fdt_prop *prop) {

    if (!strncmp(n_name, "chosen", 7) && !strncmp(p_name, "linux,initrd-start", 19)) {
        DEVTREE_CPIO_BASE = (void*)((long unsigned int) fdt32_to_cpu(*((unsigned int*)(prop + 1))));
    }
    if (!strncmp(n_name, "chosen", 7) && !strncmp(p_name, "linux,initrd-end", 17)) {
        DEVTREE_CPIO_END = (void*)((long unsigned int) fdt32_to_cpu(*((unsigned int*)(prop + 1))));
    }

}

void cpio_ls(void) {
    struct cpio_newc_header *head =  (void*) DEVTREE_CPIO_BASE;//(struct cpio_newc_header *) DTB_CPIO_BASE;//CPIO_BASE_RPI; //(void*)CPIO_QEMU_BASE;//
    uint32_t h_size = sizeof(struct cpio_newc_header);

    while (1) {
        char *f_name = ((void*) head) + h_size;

        

        if (!strncmp(f_name, "TRAILER!!!",10))break;

        printf(f_name);
        printf("\r\n");

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
    struct cpio_newc_header *head =  (void*) DEVTREE_CPIO_BASE;//(struct cpio_newc_header *) DTB_CPIO_BASE;//CPIO_BASE_QEMU; //CPIO_BASE_RPI;//(void*)CPIO_QEMU_BASE;//

    char input[BUFSIZE];

    printf("Filename: ");
    char c ='\0';
    int ind = 0;
    while (1) {
        c = uart_recv();
        if (c == '\r' || c == '\n') {
            uart_send_string("\n");
            
            if (ind < 256) input[ind] = '\0';
            else input[255] = '\0';
            
            break;
        } else {
            uart_send(c);
            input[ind++] = c;
        } 
    }
    

    while (1) {
        char *f_name = ((void*) head) + h_size;

        if (strncmp(f_name, CPIO_FOOTER_MAGIC, 10) == 0) {
            printf("No matching file\r\n"); 
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
                    printf("\r\n");
                } else {
                    uart_send(content[i]);
                }
            }

            if (content[f_size - 1] != '\n') {
                printf("\r\n");
            }
            break;
        }

        if(f_size % 4 != 0) f_size = 4 * ((f_size + 4) / 4);
        head  = ((void*) head) + offset + f_size;
    }


}

void cpio_exec(void) {
    uint32_t h_size = sizeof(struct cpio_newc_header);
    struct cpio_newc_header *head = (void*) DEVTREE_CPIO_BASE;//(struct cpio_newc_header *) DTB_CPIO_BASE;//CPIO_BASE_QEMU; //CPIO_BASE_RPI;//

    char input[BUFSIZE];

    printf("Filename: ");
    char c ='\0';
    int ind = 0;
    while (1) {
        c = uart_recv();
        if (c == '\r' || c == '\n') {
            uart_send_string("\n");
            
            if (ind < 256) input[ind] = '\0';
            else input[255] = '\0';
            
            break;
        } else {
            uart_send(c);
            input[ind++] = c;
        } 
    }    
    int no_match=1;

    while (1) {
        char *f_name = ((void*) head) + h_size;
        
        if (strncmp(f_name, CPIO_FOOTER_MAGIC, 10) == 0) {
            printf("No matching file\r\n"); 
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

    if(no_match)printf("No matching file\r\n");

}

