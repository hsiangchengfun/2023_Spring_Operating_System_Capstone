#ifndef	_P_uart_H
#define	_P_uart_H

#include "peripherals/base.h"


#define AUX_MU_MSR_REG  ((volatile unsigned int*)(MMIO_BASE+0x00215058))
#define AUX_MU_SCRATCH  ((volatile unsigned int*)(MMIO_BASE+0x0021505C))
#define AUX_MU_STAT_REG ((volatile unsigned int*)(MMIO_BASE+0x00215064))
#define AUX_MU_BAUD_REG ((volatile unsigned int*)(MMIO_BASE+0x00215068))


#define MMIO_BASE 0x3F000000

#define AUX_ENABLES      ((volatile unsigned int*)(MMIO_BASE + 0x215004))
#define AUX_MU_IO_REG   ((volatile unsigned int*)(MMIO_BASE + 0x215040))
#define AUX_MU_IER_REG  ((volatile unsigned int*)(MMIO_BASE + 0x215044))
#define AUX_MU_IIR_REG  ((volatile unsigned int*)(MMIO_BASE + 0x215048))
#define AUX_MU_LCR_REG  ((volatile unsigned int*)(MMIO_BASE + 0x21504C))
#define AUX_MU_MCR_REG  ((volatile unsigned int*)(MMIO_BASE + 0x215050))
#define AUX_MU_LSR_REG  ((volatile unsigned int*)(MMIO_BASE + 0x215054))
#define AUX_MU_CNTL_REG ((volatile unsigned int*)(MMIO_BASE + 0x215060))
#define AUX_MU_BAUD     ((volatile unsigned int*)(MMIO_BASE + 0x215068))

#endif  /*_P_uart_H */