#include "cpio.h"
#include "mini_uart.h"
#include "sched.h"
#include "fork.h"
#include "string.h"
#include "mmu.h"
#include "utils.h"

void *DEVTREE_CPIO_BASE = 0;
void *DEVTREE_CPIO_END = 0;


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


void cpio_ls() {

    struct cpio_newc_header *header;
    uint32_t filesize;
    uint32_t namesize;
    uint32_t offset;
    char *filename;

    header = DEVTREE_CPIO_BASE;

    while (1) {
        
        filename = ((void*)header) + sizeof(struct cpio_newc_header);
        
        if (stringncmp((char*)header, CPIO_HEADER_MAGIC, 6) != 0) {
            uart_send_string("invalid magic\n");
            break;
        }
        if (stringncmp(filename, CPIO_FOOTER_MAGIC, 11) == 0) break;

        uart_send_string(filename);
        uart_send('\n');

        namesize = atoi(header->c_namesize, 8);
        filesize = atoi(header->c_filesize, 8);

        offset = sizeof(struct cpio_newc_header) + namesize;
        if (offset % 4 != 0) 
            offset = ((offset/4) + 1) * 4;
        
        if (filesize % 4 != 0)
            filesize = ((filesize/4) + 1) * 4;

        offset = offset + filesize;

        header = ((void*)header) + offset;
        
    }
    
}

void cpio_cat() {

    char input[256];
    char c = '\0';
    int idx = 0;

    struct cpio_newc_header *header;
    uint32_t filesize;
    uint32_t namesize;
    uint32_t offset;
    char *filename;

    header = DEVTREE_CPIO_BASE;

    uart_send_string("Filename: ");

    while (1) {
        c = uart_recv();
        if (c == '\r' || c == '\n') {
            uart_send_string("\n");
            
            if (idx < 256) input[idx] = '\0';
            else input[255] = '\0';
            
            break;
        } else {
            uart_send(c);
            input[idx++] = c;
        } 
    }

    while (1) {
        
        filename = ((void*)header) + sizeof(struct cpio_newc_header);
        
        if (stringncmp((char*)header, CPIO_HEADER_MAGIC, 6) != 0) {
            uart_send_string("invalid magic\n");
            break;
        }
        if (stringncmp(filename, CPIO_FOOTER_MAGIC, 11) == 0) {
            uart_send_string("file does not exist!\n");
            break;
        }

        namesize = atoi(header->c_namesize, 8);
        
        offset = sizeof(struct cpio_newc_header) + namesize;
        if (offset % 4 != 0) 
            offset = ((offset/4) + 1) * 4;

        filesize = atoi(header->c_filesize, 8);

        if (stringncmp(filename, input, namesize) == 0) {
            
            char *content = ((void*)header) + offset;

            for (int i=0; i<filesize; i++) {
                uart_send(content[i]);
            }
            
            if (content[filesize-1] != '\n') uart_send('\n');

            break;
        }

        if (filesize % 4 != 0)
            filesize = ((filesize/4) + 1) * 4;

        offset = offset + filesize;

        header = ((void*)header) + offset;
        
    }

}

void cpio_exec() {

    char input[256];
    char c = '\0';
    int idx = 0;

    struct cpio_newc_header *header;
    uint32_t filesize;
    uint32_t namesize;
    uint32_t offset;
    char *filename;
    uint32_t h_size = sizeof(struct cpio_newc_header);

    header = DEVTREE_CPIO_BASE;

    uart_send_string("Execute file: ");

    while (1) {
        c = uart_recv();
        if (c == '\r' || c == '\n') {
            uart_send_string("\n");
            
            if (idx < 256) input[idx] = '\0';
            else input[255] = '\0';
            
            break;
        } else {
            uart_send(c);
            input[idx++] = c;
        } 
    }

    while (1) {
        
        filename = ((void*)header) + h_size;
        
        if (stringncmp((char*)header, CPIO_HEADER_MAGIC, 6) != 0) {
            uart_send_string("invalid magic\n");
            break;
        }
        if (stringncmp(filename, CPIO_FOOTER_MAGIC, 11) == 0) {
            uart_send_string("file does not exist!\n");
            break;
        }

        namesize = atoi(header->c_namesize, 8);
        
        offset = h_size + namesize;

        if(offset % 4 != 0) offset = 4 * ((offset + 4) / 4);


        filesize = atoi(header->c_filesize, 8);

        if (stringncmp(filename, input, namesize) == 0) {

            char *code_loc = ((void*)header) + offset;
            uint32_t sp_val = 0x600000;
            
            asm volatile(
                "msr elr_el1, %0\n\t"
                "msr spsr_el1, xzr\n\t"
                "msr sp_el0, %1\n\t"
                "eret\n\t"
                :: 
                "r" (code_loc),
                "r" (sp_val)
            );

            break;
        }

        if(filesize % 4 != 0) filesize = 4 * ((filesize + 4) / 4);

        offset = offset + filesize;

        header = ((void*)header) + offset;
        
    }

}



void initramfs_callback(char *node_name, char *prop_name, struct fdt_prop *prop) {

    if (stringncmp(node_name, "chosen", 7) == 0 && stringncmp(prop_name, "linux,initrd-start", 19) == 0) {

        DEVTREE_CPIO_BASE = (void*)fdt32_to_cpu(*((uint32_t*)(prop + 1))) + VA_START;

    }
    if (!stringncmp(node_name, "chosen", 7) && !stringncmp(prop_name, "linux,initrd-end", 17)) {
        DEVTREE_CPIO_END = (void*)(fdt32_to_cpu(*((uint32_t*)(prop + 1)))) + VA_START;
    }


}