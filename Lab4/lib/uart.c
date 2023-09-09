#include "gpio.h"
#include "uart.h"
#include "utils.h"
#include "exception.h"

char read_buf[BUFSIZE];
char write_buf[BUFSIZE];
uint32_t read_head = 0, read_end = 0;
uint32_t write_head = 0, write_end = 0;


void mini_uart_init(void) {
    // configure register to change alternate function
    *GPFSEL1 &= ~((0x7 << 12) | (0x7 << 15));
    *GPFSEL1 |=  ((0x2 << 12) | (0x2 << 15));
    
    // configure pull up/down register to disable GPIO pull up/down
    *GPPUD = 0; delay(150);
    *GPPUDCLK0 = (1 << 14) || (1 << 15); delay(150);

    *AUX_ENABLE      |= 1;      // enable mini UART and its register can be accessed
    *AUX_MU_CNTL_REG  = 0;      // disable transmitter and receiver during configuration
    *AUX_MU_IER_REG   = 0;      // disable interrupt because currently do not need it
    *AUX_MU_LCR_REG   = 3;      // set the data size to 8 bit
    *AUX_MU_MCR_REG   = 0;      // do not need auto flow control
    *AUX_MU_BAUD      = 270;    // set baud rate to 115200
    *AUX_MU_IIR_REG   = 6;      // no FIFO
    *AUX_MU_CNTL_REG  = 3;      // enable the transmitter and recevier
}

char mini_uart_getc(void) {
    while ((*AUX_MU_LSR_REG & 0x01) == 0);
    return (char) *AUX_MU_IO_REG;
}


char mini_uart_recv() {
  while (1) {
    if ((*AUX_MU_LSR_REG)&0x01) break;
  }
  return (*AUX_MU_IO_REG)&0xFF;
}

void mini_uart_gets(char *buffer, unsigned int size) {
    char *p = buffer;

    for (int i = 0; i < size - 1; i++) {
        char c = mini_uart_getc();
        if (c == '\r' || c == '\n') {
            mini_uart_puts("\r\n"); break;
        } else if (c < 31 || c > 128) {
            continue;
        }
        mini_uart_putc(c); *p++ = c;
    }

    *p = '\0';
}

void mini_uart_putc(const char c) {
    while ((*AUX_MU_LSR_REG & 0x20) == 0);
    *AUX_MU_IO_REG = c;
}

void mini_uart_puts(const char *s) {
    while (*s != '\0') {
        mini_uart_putc(*s++);
    }
}







//=======================================================

void enable_uart_interrupt() {
    enable_read_interrupt();
    *ENABLE_IRQS_1 |= (1 << 29); //interrupt controller’s Enable IRQs1(0x3f00b210)’s bit29.
                                //BCM2837 SPEC P112 table
                                //0x210 enable IRQs1
}

void disable_uart_interrupt() {
    disable_write_interrupt();
    *DISABLE_IRQS_1 |= (1 << 29);
                                //BCM2837 SPEC P112 table
                                //0x21C disable IRQs1
}





//AUX_MU_IER_REG  bit 0 => set for read  RX
//                bit 1 => set for write  TX

void enable_read_interrupt() {
    *AUX_MU_IER_REG |= 0x01;
}

void disable_read_interrupt() {
    *AUX_MU_IER_REG &= ~0x01;
}

void enable_write_interrupt() {
    *AUX_MU_IER_REG |= 0x02;
}

void disable_write_interrupt() {
    *AUX_MU_IER_REG &= ~0x02;
}




void async_uart_handler() {
    disable_uart_interrupt();
    //The AUX_MU_IIR_REG register shows the interrupt status. 
    if (*AUX_MU_IIR_REG & 0x04) { // 100 //read mode   Receiver holds valid byte 
    
        char c = *AUX_MU_IO_REG&0xFF;
        read_buf[read_end++] = c;
        if(read_end==BUFSIZE)read_end=0;

    
    } else if (*AUX_MU_IIR_REG & 0x02) { //010  //send mode // check write enabled // Transmit holding register empty

        
        while (*AUX_MU_LSR_REG & 0x20) { //0010 0000 //Both bits [7:6] always read as 1 as the FIFOs are always enabled 
            if (write_head == write_end) {             
                    enable_read_interrupt(); 
                    break;
                }
            char c = write_buf[write_head];
            write_head++;
            *AUX_MU_IO_REG = c;
            if(write_head==BUFSIZE)write_head=0;
        }
    }

    enable_uart_interrupt();
}



uint32_t async_uart_gets(char *input, uint32_t size) {
    
    int len=0;
    for (len = 0; len < size - 1; len++) {
        while (read_head == read_end) asm volatile("nop \r\n");
        

        if (read_buf[read_head] == '\r' || read_buf[read_head] == '\n') {
            read_head++;
            if(read_head >= BUFSIZE -1 ) read_head -= BUFSIZE;
            break;
        }

        input[len] = read_buf[read_head];
        read_head++;
        if(read_head==BUFSIZE)read_head=0;
    }

    input[len] = '\0';

    return len;
}



void async_uart_puts(const char *s) {
    while (*s != '\0') {
        write_buf[write_end++] = *s++;
        if(write_end==BUFSIZE)write_end=0;
    }
    enable_write_interrupt();
}

void delay(int time) {
    while (time--);
}
