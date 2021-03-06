#include "uart0.h"
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



void uart0_init (void)
{
    /* Configure UART0 */
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
    NFEN0.nfen0 |= 1;                                       /* Enable noise filter on RxD0 pin */
    SMR00.smr00 = 0x0023U;                                  /* CKS=0   Operation clock : CK00,
                                                               CSS=0   Transfer clock : division of CK00
                                                               MD21=01 Operation mode : UART
                                                               MD0=1   Buffer empty interrupt
                                                            */
    SCR00.scr00 = 0x8097U;                                  /* TXE=1 RXE=0 Transmission only
                                                               EOC=0       Reception error interrupt masked
                                                               DAP=0 CKP=0 Phase clock : type 1
                                                               PTC=0       No parity
                                                               DIR=1       LSB first
                                                               SLC=1       1 stop bit
                                                               DLS=3       8-bit data length
                                                            */
    SDR00.sdr00 = SDR_VALUE << 9;                           /* transfer clock : 16 MHz / (7 + 1) / 2 = 1 Mbps */
    /* Setup operation mode for UART0 receiver */
    SMR01.smr01 = 0x0122U;                                  /* CKS=0   Operation clock : CK00,
                                                               CCS=0   Transfer clock : division of CK00
                                                               STS=1   Detect falling edge as start bit
                                                               SIS=0   Normal reception
                                                               MD21=01 Operation mode : UART
                                                               MD0=0   Transfer end interrupt
                                                            */
    SCR01.scr01 = 0x4497U;                                  /* TXE=0 RXE=1 Reception only
                                                               EOC=1       Reception error interrupt enabled
                                                               DAP=0 CKP=0 Phase clock : type 1
                                                               PTC=0       No parity
                                                               DIR=1       LSB first
                                                               SLC=1       1 stop bit
                                                               DLS=3       8-bit data length
                                                            */
    SDR01.sdr01 = SDR_VALUE << 9;                           /* transfer clock : 16 MHz / (7 + 1) / 2 = 1 Mbps */

    SOL0.sol0 &= ~0x01U;                                    /* Output is not inverted */
    SO0.so0   |=  0x01U;                                    /* Initial output level is 1 */
    SOE0.soe0 |=  0x01U;                                    /* Enable data output */

    /* Configure P12 as open-drain output */
    POM1.pom1 |=  (1U << 2);
    P1.p1   |=  (1U << 2);
    PM1.pm1  &= ~(1U << 2);

    /* Configure P11 as digital input with built-in pull-up */
    PM1.pm1  |=  (1U << 1);
    POM1.pom1 &= ~(1U << 1);
    PIM1.pim1 &= ~(1U << 1);
    PU1.pu1  |=  (1U << 1);

    /* Enable UART0 operation */
	SS0.ss0 |= (1U << 0) | (1U << 1);

    /* Mask all UART0 interrupts */
    STMK0  = 1;
    SRMK0  = 1;
    SREMK0 = 1;

    /* Setup interrupt flags with initial values */
    STIF0  = 1;
    SRIF0  = 0;
    SREIF0 = 0;
}

int uart0_putchar (int c)
{
    while (0 == STIF0);
    STIF0 = 0;
    SDR00.sdr00 = (unsigned char)c;
    return (unsigned char)c;
}

int uart0_puts (const char * s)
{
    while ('\0' != *s)
    {
        (void)uart0_putchar(*s++);
    }
    (void)uart0_putchar('\n');
    return 1;
}

void uart0_flush (void)
{
    /* Discard any data in the input buffer */
    SRIF0 = 0;
}

int uart0_getchar (void)
{
    while (0 == SRIF0 && 0 == SREIF0);
    SRIF0 = 0;
    SREIF0 = 0;

    int c = EOF;
    unsigned int status = SSR01.ssr01;
    if (status & (1U << 5))                                  /* BFF01 == 1: If valid data present */
    {
        c = (unsigned char)SDR01.sdr01;
    }
    if (status & 0x07)
    {
        SIR01.sir01 = status; // Serial Flag Clear Trigger Register mn (SIRmn)
		// FEF (framing) - 2, PEF (Parity/ACK) - 1, OVF (overrun) - 0.
        if (status & (1U << 2))                              /* FEF01 == 1: Framing error occurred */
        {
			c = 0x66U; // f
            ST0.st0 = (1U << 1);                             /* Disable UART0 receiver operation (channel 1) */
            while (SE0.se0 & (1U << 1));
            SS0.ss0 = (1U << 1);                             /* Enable UART0 receiver operation (channel 1) */
        }
        if (status & (1U << 1))                              /* PEF01 == 1: Parity/ACK error occurred */
        {
			c = 0x70U; //p
        }
        if (status & (1U << 0))                              /* OVF01 == 1: overrun error occurred */
        {
			c = 0x6fU;  //o
        }
    }
    return c; //only for debug !!
}

char* uart0_gets (char * s, int size)
{
    char *p = s;
    int len = 0;

    while (len < size)
    {
        int c = uart0_getchar();
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

__attribute__((interrupt))
void st0_handler(void)
{
}

__attribute__((interrupt))
void sr0_handler(void)
{
}

/* This is actually INTSRE0 interrupt handler */
__attribute__((interrupt))
void tm01h_handler(void)
{
}
