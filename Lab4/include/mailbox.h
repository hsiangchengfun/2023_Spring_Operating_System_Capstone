#ifndef __MAILBOX_H__
#define __MAILBOX_H__

#include "utils.h"
#define MMIO_BASE 0x3F000000

#define MAILBOX_BASE        (MMIO_BASE + 0xB880)
#define MAILBOX_READ        ((volatile unsigned int*)(MAILBOX_BASE))
#define MAILBOX_STATUS      ((volatile unsigned int*)(MAILBOX_BASE + 0x18))
#define MAILBOX_WRTIE       ((volatile unsigned int*)(MAILBOX_BASE + 0x20))



#define GET_BOARD_REVISION  0x00010002
#define GET_ARM_MEMORY      0x00010005
#define REQUEST_CODE        0x00000000
#define REQUEST_SUCCEED     0x80000000
#define REQUEST_FAILED      0x80000001
#define TAG_REQUEST_CODE    0x00000000
#define END_TAG             0x00000000
#define MAILBOX_EMPTY       0x40000000
#define MAILBOX_FULL        0x80000000



int mailbox_call(uint32_t *msg);
void get_board_revision();
void get_memory_info();

#endif