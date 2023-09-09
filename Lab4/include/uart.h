#ifndef __UART_H__
#define __UART_H__
#include "utils.h"
#define MMIO_BASE 0x3F000000

#define AUX_ENABLE      ((volatile unsigned int*)(MMIO_BASE + 0x215004))
#define AUX_MU_IO_REG   ((volatile unsigned int*)(MMIO_BASE + 0x215040))
#define AUX_MU_IER_REG  ((volatile unsigned int*)(MMIO_BASE + 0x215044))
#define AUX_MU_IIR_REG  ((volatile unsigned int*)(MMIO_BASE + 0x215048))
#define AUX_MU_LCR_REG  ((volatile unsigned int*)(MMIO_BASE + 0x21504C))
#define AUX_MU_MCR_REG  ((volatile unsigned int*)(MMIO_BASE + 0x215050))
#define AUX_MU_LSR_REG  ((volatile unsigned int*)(MMIO_BASE + 0x215054))
#define AUX_MU_CNTL_REG ((volatile unsigned int*)(MMIO_BASE + 0x215060))
#define AUX_MU_BAUD     ((volatile unsigned int*)(MMIO_BASE + 0x215068))


char mini_uart_recv();
void delay(int time);
void mini_uart_init(void);
char mini_uart_getc(void);
void mini_uart_gets(char *buffer, unsigned int size);
void mini_uart_putc(const char  c);
void mini_uart_puts(const char *s);


void enable_read_interrupt(void);
void disable_read_interrupt(void);
void enable_write_interrupt(void);
void disable_write_interrupt(void);
void enable_uart_interrupt(void);
void disable_uart_interrupt(void);
void async_uart_handler(void);
void async_uart_puts(const char *s);
uint32_t async_uart_gets(char *buffer, uint32_t size);

#endif