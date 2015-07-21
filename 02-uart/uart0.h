#ifndef UART0_H__
#define UART0_H__

void uart0_init (void);
void uart0_flush (void);
int uart0_putchar (int c);
int uart0_getchar (void);
int uart0_puts (const char * s);
char* uart0_gets (char * s, int size);

#define EOF (-1)

#endif /* UART0_H__ */

