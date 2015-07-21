#include "uart2.h"
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



void uart2_init (void)
{
    /* Configure UART2 */
    SAU1EN = 1;                                             /* Supply clock to serial array unit 0 */
    {
        /* >=4 cycle delay required by manual */
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
    }
    SPS1.sps1 = (PRS_VALUE << 4) | PRS_VALUE;               /* Set input clock (CK00 and CK01) to fclk = 16MHz */
    /* Setup operation mode for UART2 transmitter */
    SMR10.smr10 = 0x0023U;                                  /* CKS=0   Operation clock : CK00,
                                                               CSS=0   Transfer clock : division of CK00
                                                               MD21=01 Operation mode : UART
                                                               MD0=1   Buffer empty interrupt
                                                            */
    SCR10.scr10 = 0x8097U;                                  /* TXE=1 RXE=0 Transmission only
                                                               EOC=0       Reception error interrupt masked
                                                               DAP=0 CKP=0 Phase clock : type 1
                                                               PTC=0       No parity
                                                               DIR=1       LSB first
                                                               SLC=1       1 stop bit
                                                               DLS=3       8-bit data length
                                                            */
    SDR10.sdr10 = SDR_VALUE << 9;                           /* transfer clock : 16 MHz / (7 + 1) / 2 = 1 Mbps */
    /* Setup operation mode for UART2 receiver */
    SMR11.smr11 = 0x0122U;                                  /* CKS=0   Operation clock : CK00,
                                                               CCS=0   Transfer clock : division of CK00
                                                               STS=1   Detect falling edge as start bit
                                                               SIS=0   Normal reception
                                                               MD21=01 Operation mode : UART
                                                               MD0=0   Transfer end interrupt
                                                            */
    SCR11.scr11 = 0x4497U;                                  /* TXE=0 RXE=1 Reception only
                                                               EOC=1       Reception error interrupt enabled
                                                               DAP=0 CKP=0 Phase clock : type 1
                                                               PTC=0       No parity
                                                               DIR=1       LSB first
                                                               SLC=1       1 stop bit
                                                               DLS=3       8-bit data length
                                                            */
    SDR11.sdr11 = SDR_VALUE << 9;                                /* transfer clock : 16 MHz / (7 + 1) / 2 = 1 Mbps */

    SOL1.sol1 &= ~0x01U;                                    /* Output is not inverted */
    SO1.so1   |=  0x01U;                                    /* Initial output level is 1 */
    SOE1.soe1 |=  0x01U;                                    /* Enable data output */

    /* Configure P13 as open-drain output */
    POM1.pom1 |=  (1U << 3);
    P1.p1     |=  (1U << 3);
    PM1.pm1   &= ~(1U << 3);

    /* Configure P14 as digital input with built-in pull-up */
    PM1.pm1   |=  (1U << 4);
    POM1.pom1 &= ~(1U << 4);
    PIM1.pim1 &= ~(1U << 4);
    PU1.pu1   |=  (1U << 4);

    /* Enable UART2 operation */
	SS1.ss1 |= 0x01U | 0x02U;

    /* Mask all UART2 interrupts */
    STMK2  = 1;
    SRMK2  = 1;
    SREMK2 = 1;

    /* Setup interrupt flags with initial values */
    STIF2  = 1;
    SRIF2  = 0;
    SREIF2 = 0;
}

int uart2_putchar (int c)
{
    while (0 == STIF2);
    STIF2 = 0;
    SDR10.sdr10 = (unsigned char)c;
    return (unsigned char)c;
}

int uart2_puts (const char * s)
{
    while ('\0' != *s)
    {
        (void)uart2_putchar(*s++);
    }
    (void)uart2_putchar('\n');
    return 1;
}

void uart2_flush (void)
{
    /* Discard any data in the input buffer */
    SRIF2 = 0;
}

int uart2_getchar (void)
{
    while (0 == SRIF2 && 0 == SREIF2);
    SRIF2 = 0;
    SREIF2 = 0;

    int c = EOF;
    unsigned int status = SSR11.ssr11;
    if (status & (1U << 5))                                  /* BFF01 == 1: If valid data present */
    {
        c = (unsigned char)SDR11.sdr11;
    }
    if (status & 0x07)
    {
        SIR11.sir11 = status;
        if (status & (1U << 2))                              /* FEF01 == 1: Framing error occurred */
        {
            ST1.st1 = (1U << 1);                             /* Disable UART2 receiver operation (channel 1) */
            while (SE1.se1 & (1U << 1));
            SS1.ss1 = (1U << 1);                             /* Enable UART2 receiver operation (channel 1) */
        }
    }
    return c;
}

char* uart2_gets (char * s, int size)
{
    char *p = s;
    int len = 0;

    while (len < size)
    {
        int c = uart2_getchar();
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
