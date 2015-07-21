#ifndef UART3_H__
#define UART3_H__

void uart3_init (void);
void uart3_flush (void);
int uart3_putchar (int c);
int uart3_getchar (void);
int uart3_puts (const char * s);
char* uart3_gets (char * s, int size);

#define EOF (-1)

#endif /* UART2_H__ */

