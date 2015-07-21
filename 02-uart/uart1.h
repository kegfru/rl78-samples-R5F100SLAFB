#ifndef UART1_H__
#define UART1_H__

void uart1_init (void);
void uart1_flush (void);
int uart1_putchar (int c);
int uart1_getchar (void);
int uart1_puts (const char * s);
char* uart1_gets (char * s, int size);

#define EOF (-1)

#endif /* UART1_H__ */

