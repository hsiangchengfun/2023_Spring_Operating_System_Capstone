#ifndef __DEVTREE_H__
#define __DEVTREE_H__

#define FDT_HEADER_MAGIC    0xd00dfeed
#define FDT_BEGIN_NODE      0x00000001
#define FDT_END_NODE        0x00000002
#define FDT_PROP            0x00000003
#define FDT_NOP             0x00000004
#define FDT_END             0x00000009

#include "utils.h"

struct fdt_header {
    uint32_t magic;             // the value 0xd00dfeed (big-endian)
    uint32_t totalsize;         // the total size in bytes of the devicetree
    uint32_t off_dt_struct;     // the offset in bytes of the structure block from the beginning of the header
    uint32_t off_dt_strings;    // the offset in bytes of the strings block from the beginning of the header
    uint32_t off_mem_rsvmap;    // the offset in bytes of the memory reservation block from the beginning of the header
    uint32_t version;           // the version of the devicetree data structure
    uint32_t last_comp_version; // the lowest version of the devicetree data structure with which the version used in backwards compatible
    uint32_t boot_cpuid_phys;   // the physical ID of the system's boot CPU
    uint32_t size_dt_strings;   // the length in bytes of the strings block section of the devicetree blob
    uint32_t size_dt_struct;    // the length in bytes of the structure block section of the devicetree blob
};

struct fdt_prop {
    uint32_t len;               // the length of the property's value in bytes (big-endian)
    uint32_t nameoff;           // an offset into the strings block at which the property's name is stored as a null-terminated string (big-endian)
};

// void devicetree_get_address(void);
void fdt_traverse(void (*callback)(char*, char*, struct fdt_prop*));
uint32_t fdt32_to_cpu(uint32_t fdt_num); // big to little endian

#endif