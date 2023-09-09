#ifndef __CPIO_H__
#define __CPIO_H__


#define CPIO_HEADER_MAGIC       "070701"
#define CPIO_FOOTER_MAGIC       "TRAILER!!!"

#define PI_CPIO_BASE            ((void*) (0x20000000))
#define QEMU_CPIO_BASE          ((void*) (0x8000000))

#include "devtree.h"
#include "utils.h"

#define STACKSIZE 0x2000

extern void *DEVTREE_CPIO_BASE;

struct cpio_newc_header {
    char c_magic[6];        // The string "070701"
    char c_ino[8];
    char c_mode[8];
    char c_uid[8];
    char c_gid[8];
    char c_nlink[8];
    char c_mtime[8];
    char c_filesize[8];
    char c_devmajor[8];
    char c_devminor[8];
    char c_rdevmajor[8];
    char c_rdevminor[8];
    char c_namesize[8];
    char c_check[8];        // always set to zero by writers and ignored by readers
};

void initramfs_callback (char *, char *, struct fdt_prop *);
void cpio_ls ();
void cpio_cat ();
void cpio_exec ();
uint32_t atoi(char *s, uint32_t len);
#endif