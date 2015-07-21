#include "uart1.h"
#include <iodefine.h>
#include <iodefine_ext.h>
#include <stddef.h>

#define f_CLK         			32000000 // 32 MHz
#define DESIRED_BAUDRATE 		9600

/* Note that only 9600, 38400, and 115200 bps were tested. */
#define PRESCALE_THRESH  		((38400 + 115200) / 2)
#define PRS_VALUE        		((DESIRED_BAUDRATE < PRESCALE_THRESH) ? 4 : 0)
#define f_MCK            		(f_CLK / (1 << PRS_VALUE))
#define SDR_VALUE        		(f_MCK / DESIRED_BAUDRATE / 2 - 1)



void uart1_init (void)
{
    /* Configure UART1 */
    SAU0EN = 1;                                             /* Supply clock to serial array unit 0 */
    {
        /* >=4 cycle delay required by manual */
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
    }
    SPS0.sps0 = (PRS_VALUE << 4) | PRS_VALUE;               /* Set input clock (CK00 and CK01) to fclk = 16MHz */
    /* Setup operation mode for UART0 transmitter */
    SMR02.smr02 = 0x0023U;                                  /* CKS=0   Operation clock : CK00,
                                                               CSS=0   Transfer clock : division of CK00
                                                               MD21=01 Operation mode : UART
                                                               MD0=1   Buffer empty interrupt
                                                            */
    SCR02.scr02 = 0x8097U;                                  /* TXE=1 RXE=0 Transmission only
                                                               EOC=0       Reception error interrupt masked
                                                               DAP=0 CKP=0 Phase clock : type 1
                                                               PTC=0       No parity
                                                               DIR=1       LSB first
                                                               SLC=1       1 stop bit
                                                               DLS=3       8-bit data length
                                                            */
    SDR02.sdr02 = SDR_VALUE << 9;                           /* transfer clock : 16 MHz / (7 + 1) / 2 = 1 Mbps */
    /* Setup operation mode for UART0 receiver */
    SMR03.smr03 = 0x0122U;                                  /* CKS=0   Operation clock : CK00,
                                                               CCS=0   Transfer clock : division of CK00
                                                               STS=1   Detect falling edge as start bit
                                                               SIS=0   Normal reception
                                                               MD21=01 Operation mode : UART
                                                               MD0=0   Transfer end interrupt
                                                            */
    SCR03.scr03 = 0x4497U;                                  /* TXE=0 RXE=1 Reception only
                                                               EOC=1       Reception error interrupt enabled
                                                               DAP=0 CKP=0 Phase clock : type 1
                                                               PTC=0       No parity
                                                               DIR=1       LSB first
                                                               SLC=1       1 stop bit
                                                               DLS=3       8-bit data length
                                                            */
    SDR03.sdr03 = SDR_VALUE << 9;                           /* transfer clock : 16 MHz / (7 + 1) / 2 = 1 Mbps */

    SOL0.sol0 &= ~0x04U;                                    /* Output is not inverted */
    SO0.so0   |=  0x04U;                                    /* Initial output level is 1 */
    SOE0.soe0 |=  0x04U;                                    /* Enable data output */

    /* Configure P02 as open-drain output */
    POM0.pom0 |=  (1U << 2);
    P0.p0   |=  (1U << 2);
    PM0.pm0  &= ~(1U << 2);
	PMC0.pmc0 &= 0xFBU;
    /* Configure P03 as digital input with built-in pull-up */
    PM0.pm0  |=  (1U << 3);
    POM0.pom0 &= ~(1U << 3);
    PIM0.pim0 &= ~(1U << 3);
    PU0.pu0  |=  (1U << 3);
	PMC0.pmc0 &= 0xF7U;
    /* Enable UART0 operation */
	SS0.ss0 |= 0x04U | 0x08U;


    /* Mask all UART0 interrupts */
    STMK1  = 1;
    SRMK1  = 1;
    SREMK1 = 1;

    /* Setup interrupt flags with initial values */
    STIF1  = 1;
    SRIF1  = 0;
    SREIF1 = 0;
}

int uart1_putchar (int c)
{
    while (0 == STIF1);
    STIF1 = 0;
    SDR02.sdr02 = (unsigned char)c;
    return (unsigned char)c;
}

int uart1_puts (const char * s)
{
    while ('\0' != *s)
    {
        (void)uart1_putchar(*s++);
    }
    (void)uart1_putchar('\n');
    return 1;
}

void uart1_flush (void)
{
    /* Discard any data in the input buffer */
    SRIF1 = 0;
}

int uart1_getchar (void)
{
    while (0 == SRIF1 && 0 == SREIF1);
    SRIF1 = 0;
    SREIF1 = 0;

    int c = EOF;
    unsigned int status = SSR03.ssr03;
    if (status & (1U << 5))                                  /* BFF01 == 1: If valid data present */
    {
        c = (unsigned char)SDR03.sdr03;
    }
    if (status & 0x07)
    {
        SIR03.sir03 = status;
        if (status & (1U << 2))                              /* FEF01 == 1: Framing error occurred */
        {
            ST0.st0 = (1U << 1);                             /* Disable UART0 receiver operation (channel 1) */
            while (SE0.se0 & (1U << 1));
            SS0.ss0 = (1U << 1);                             /* Enable UART0 receiver operation (channel 1) */
        }
    }
    return c;
}

char* uart1_gets (char * s, int size)
{
    char *p = s;
    int len = 0;

    while (len < size)
    {
        int c = uart1_getchar();
        if (EOF == c)
        {
            *p = '\0';
            return NULL;
        }
        *p++ = c;
        if ('\n' == c)
        {
            break;
        }
    }
    *p = '\0';
    return s;
}
