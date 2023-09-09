#ifndef __CPIO_H__
#define __CPIO_H__

#include "devtree.h"


#define CPIO_BASE_QEMU  (0x8000000)
#define CPIO_BASE_RPI   (0x20000000)

struct cpio_newc_header {
    char c_magic[6];        // The string 070701 for new ASCII
    char c_ino[8];
    char c_mode[8];
    char c_uid[8];
    char c_gid[8];
    char c_nlink[8];
    char c_mtime[8];
    char c_filesize[8];     //must be 0 for FIFOs and directories
    char c_devmajor[8];
    char c_devminor[8];
    char c_rdevmajor[8];
    char c_rdevminor[8];
    char c_namesize[8];        //count includes terminating NUL in pathname
    char c_check[8];        // 0 for "new" portable format; for CRC format the sum of all the bytes in the file
};


void initramfs_callback(char *node_name, char *prop_name, struct fdt_prop* prop);

void cpio_ls();
void cpio_cat();

#endif