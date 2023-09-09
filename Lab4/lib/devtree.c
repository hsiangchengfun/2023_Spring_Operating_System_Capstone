#include "uart.h"
#include "utils.h"
#include "devtree.h"

void *DEVICETREE_ADDRESS = 0;




void fdt_traverse(void (*callback)(char*, char*, struct fdt_prop*)) {
    
    asm volatile("mov %0, x20" :"=r"(DEVICETREE_ADDRESS));

    char magic[4] = {0xd0, 0x0d, 0xfe, 0xed};
    struct fdt_header *dt_header = DEVICETREE_ADDRESS;
    // mini_uart_puts((char*)magic);
    if (strncmp((char*) dt_header, magic, 4) != 0) {
        mini_uart_puts("DeviceTree magic FAILED\r\n");
        return;
    }

    void *dt_struct_addr = DEVICETREE_ADDRESS + fdt32_to_cpu(dt_header->off_dt_struct);
    char *dt_string_addr = DEVICETREE_ADDRESS + fdt32_to_cpu(dt_header->off_dt_strings);

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
            
            mini_uart_puts("token not matched\r\n");
            break;
        }
    }
}



uint32_t fdt32_to_cpu(uint32_t fdt_num) { // big to little endian
  uint8_t *part = (uint8_t*)&fdt_num;
  return (part[0] << 24) | (part[1] << 16) | (part[2] << 8) | (part[3]);
}
