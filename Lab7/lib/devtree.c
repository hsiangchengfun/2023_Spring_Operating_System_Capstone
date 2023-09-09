#include "devtree.h"
#include "mini_uart.h"
#include "utils.h"
#include "string.h"

void *DEVTREE_ADDRESS = 0;

unsigned int to_lendian(unsigned int n) {
    return ((n>>24)&0x000000FF) |
           ((n>>8) &0x0000FF00) |
           ((n<<8) &0x00FF0000) |
           ((n<<24)&0xFF000000) ;
}

uint32_t fdt32_to_cpu(uint32_t fdt_num) { // big to little endian
  uint8_t *part = (uint8_t*)&fdt_num;
  return (part[0] << 24) | (part[1] << 16) | (part[2] << 8) | (part[3]);
}

void devtree_getaddr() {
    
    asm volatile("MOV %0, x20" :  "=r"(DEVTREE_ADDRESS));

    char magic[4] = {0xd0, 0x0d, 0xfe, 0xed};
    if(strncmp((char*)DEVTREE_ADDRESS, magic, 4) != 0) {
        uart_send_string("magic failed\n");
    } else {
        uart_send_string("devtree magic succeed\n");
    }

}

void fdt_traverse(void (*callback)(char*, char*, struct fdt_prop*)) {
    
    asm volatile("mov %0, x20" :"=r"(DEVTREE_ADDRESS));

    char magic[4] = {0xd0, 0x0d, 0xfe, 0xed};
    struct fdt_header *dt_header = DEVTREE_ADDRESS;
    // mini_uart_puts((char*)magic);
    if (strncmp((char*) dt_header, magic, 4) != 0) {
        printf("DeviceTree magic FAILED\r\n");
        return;
    }

    void *dt_struct_addr = DEVTREE_ADDRESS + fdt32_to_cpu(dt_header->off_dt_struct);
    char *dt_string_addr = DEVTREE_ADDRESS + fdt32_to_cpu(dt_header->off_dt_strings);

    uint32_t offset = 0;
    char *n_name = 0;
    char *p_name = 0;

    while (1) {
        uint32_t token = fdt32_to_cpu(*((uint32_t*) dt_struct_addr));

        if (token == FDT_BEGIN_NODE) {
            n_name = dt_struct_addr + 4; // 4 is token size (32bit = 4 bytes)
            // FDT_BEGIN_NODE is followed by the node's unit name as extra data
            // print_string("Node Name:\n");
            // print_string(n_name);
            // print_char('\n');
            int n_name_size = 0;

            while(n_name[n_name_size]!='\0'){n_name_size++;}
            //get node name size
            offset = 4 + n_name_size + 1;// add one because count size is zero base
            
            //allign four byte => become nearest and bigger four base num (padding bytes)
            if(offset % 4 != 0) offset = 4 * ((offset + 4) / 4);

            dt_struct_addr += offset;

        } else if (token == FDT_END_NODE) {

            dt_struct_addr += 4;

        } else if (token == FDT_PROP) {
            
            struct fdt_prop *prop = (struct fdt_prop*)(dt_struct_addr + 4);
            
            offset = 4 + sizeof(struct fdt_prop) + fdt32_to_cpu(prop->len);
            
            if(offset % 4 != 0) offset = 4 * ((offset + 4) / 4);
            
            dt_struct_addr += offset;

            p_name = dt_string_addr + fdt32_to_cpu(prop->nameoff);
            /*
            String BLock
            The strings block contains strings representing 
            all the property names used in the tree
            */
            callback(n_name, p_name, prop);

            // print_string("Prop Name:\n");
            // print_string(p_name);
            // print_char('\n');
        } else if (token == FDT_NOP) {

            dt_struct_addr += 4;

        } else if (token == FDT_END) {
            
            dt_struct_addr += 4; break;
        } else {
            
            printf("token not matched\r\n");
            break;
        }
    }
}



