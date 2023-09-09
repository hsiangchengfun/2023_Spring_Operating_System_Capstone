#include "uart.h"
#include "utils.h"
#include "mailbox.h"

int mailbox_call(uint32_t*input) {
    unsigned long msg = ((((unsigned long) input) & ~0x0F) | (0x08 & 0x0F));
    while (*MAILBOX_STATUS & MAILBOX_FULL);
    *MAILBOX_WRTIE = msg;
    while (1) {
        while (*MAILBOX_STATUS & MAILBOX_EMPTY);
        if (*MAILBOX_READ == msg) {
            return input[1];
        }
    }
}

void get_board_revision() {
    uint32_t mailbox[7];

    mailbox[0] = 7 * 4;                             
    mailbox[1] = REQUEST_CODE;                      
    mailbox[2] = GET_BOARD_REVISION;                
    mailbox[3] = 4;                                 
    mailbox[4] = TAG_REQUEST_CODE;
    mailbox[5] = 0;                                 
    mailbox[6] = END_TAG;                           

    if (mailbox_call(mailbox) == REQUEST_SUCCEED) { 
        mini_uart_puts("Board Revision:\t\t");
        print_h(mailbox[5]);                       
        mini_uart_puts("\r\n");
    }
}

void get_memory_info(void) {
    uint32_t mailbox[8];

    mailbox[0] = 8 * 4;
    mailbox[1] = REQUEST_CODE;
    mailbox[2] = GET_ARM_MEMORY;
    mailbox[3] = 8;
    mailbox[4] = TAG_REQUEST_CODE;
    mailbox[5] = 0;
    mailbox[6] = 0;
    mailbox[7] = END_TAG;

    if (mailbox_call(mailbox) == REQUEST_SUCCEED) {
        mini_uart_puts("ARM memory base address :\t");
        print_h(mailbox[5]);
        mini_uart_puts("\r\n");
        mini_uart_puts("ARM Memory Size:\t\t");
        print_h(mailbox[6]);
        mini_uart_puts("\r\n");
    }
}