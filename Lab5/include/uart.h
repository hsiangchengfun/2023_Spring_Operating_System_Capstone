#ifndef	_uart_H
#define	_uart_H

void uart_init ( void );
char uart_recv ( void );
void uart_send ( char c );
void uart_send_string ( char* str );
void printf ( char *fmt, ... );
void mini_uart_putc(char c);
void mini_uart_puts(char *s);
char mini_uart_getc(void);
void mini_uart_gets(char *buffer, unsigned int size);

#endif  /*_uart_H */