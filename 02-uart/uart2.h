#ifndef UART2_H__
#define UART2_H__

void uart2_init (void);
void uart2_flush (void);
int uart2_putchar (int c);
int uart2_getchar (void);
int uart2_puts (const char * s);
char* uart2_gets (char * s, int size);

#define EOF (-1)

#endif /* UART2_H__ */

