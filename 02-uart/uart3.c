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



void uart3_init (void)
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
    SMR12.smr12 = 0x0023U;                                  /* CKS=0   Operation clock : CK00,
                                                               CSS=0   Transfer clock : division of CK00
                                                               MD21=01 Operation mode : UART
                                                               MD0=1   Buffer empty interrupt
                                                            */
    SCR12.scr12 = 0x8097U;                                  /* TXE=1 RXE=0 Transmission only
                                                               EOC=0       Reception error interrupt masked
                                                               DAP=0 CKP=0 Phase clock : type 1
                                                               PTC=0       No parity
                                                               DIR=1       LSB first
                                                               SLC=1       1 stop bit
                                                               DLS=3       8-bit data length
                                                            */
    SDR12.sdr12 = SDR_VALUE << 9;                           /* transfer clock : 16 MHz / (7 + 1) / 2 = 1 Mbps */
    /* Setup operation mode for UART2 receiver */
    SMR13.smr13 = 0x0122U;                                  /* CKS=0   Operation clock : CK00,
                                                               CCS=0   Transfer clock : division of CK00
                                                               STS=1   Detect falling edge as start bit
                                                               SIS=0   Normal reception
                                                               MD21=01 Operation mode : UART
                                                               MD0=0   Transfer end interrupt
                                                            */
    SCR13.scr13 = 0x4497U;                                  /* TXE=0 RXE=1 Reception only
                                                               EOC=1       Reception error interrupt enabled
                                                               DAP=0 CKP=0 Phase clock : type 1
                                                               PTC=0       No parity
                                                               DIR=1       LSB first
                                                               SLC=1       1 stop bit
                                                               DLS=3       8-bit data length
                                                            */
    SDR13.sdr13 = SDR_VALUE << 9;                                /* transfer clock : 16 MHz / (7 + 1) / 2 = 1 Mbps */

    SOL1.sol1 &= ~0x04U;                                    /* Output is not inverted */
    SO1.so1   |=  0x04U;                                    /* Initial output level is 1 */
    SOE1.soe1 |=  0x04U;                                    /* Enable data output */

    /* Configure P144 as open-drain output */
    POM14.pom14 |=  (1U << 4);
    P14.p14     |=  (1U << 4);
    PM14.pm14   &= ~(1U << 4);
	PMC14.pmc14 &= 0xEFU;
    /* Configure P143 as digital input with built-in pull-up */
    PM14.pm14   |=  (1U << 3);
    POM14.pom14 &= ~(1U << 3);
    PIM14.pim14 &= ~(1U << 3);
    PU14.pu14   |=  (1U << 3);
	PMC14.pmc14 &= 0xF7U;
    /* Enable UART2 operation */
	SS1.ss1 |= 0x04U | 0x08U;

    /* Mask all UART2 interrupts */
    STMK3  = 1;
    SRMK3  = 1;
    SREMK3 = 1;

    /* Setup interrupt flags with initial values */
    STIF3  = 1;
    SRIF3  = 0;
    SREIF3 = 0;
}

int uart3_putchar (int c)
{
    while (0 == STIF3);
    STIF3 = 0;
    SDR12.sdr12 = (unsigned char)c;
    return (unsigned char)c;
}

int uart3_puts (const char * s)
{
    while ('\0' != *s)
    {
        (void)uart3_putchar(*s++);
    }
    (void)uart3_putchar('\n');
    return 1;
}

void uart3_flush (void)
{
    /* Discard any data in the input buffer */
    SRIF3 = 0;
}

int uart3_getchar (void)
{
    while (0 == SRIF3 && 0 == SREIF3);
    SRIF3 = 0;
    SREIF3 = 0;

    int c = EOF;
    unsigned int status = SSR13.ssr13;
    if (status & (1U << 5))                                  /* BFF01 == 1: If valid data present */
    {
        c = (unsigned char)SDR13.sdr13;
    }
    if (status & 0x07)
    {
        SIR13.sir13 = status;
        if (status & (1U << 2))                              /* FEF01 == 1: Framing error occurred */
        {
            ST1.st1 = (1U << 1);                             /* Disable UART2 receiver operation (channel 1) */
            while (SE1.se1 & (1U << 1));
            SS1.ss1 = (1U << 1);                             /* Enable UART2 receiver operation (channel 1) */
        }
    }
    return c;
}

char* uart3_gets (char * s, int size)
{
    char *p = s;
    int len = 0;

    while (len < size)
    {
        int c = uart3_getchar();
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
