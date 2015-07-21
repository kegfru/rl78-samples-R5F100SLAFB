#include "QB-R5F100SL-TB.h"

void main_init(void)
{
    asm("di");
    /* Setup clocks */
    CMC.cmc = 0x11U;                                        /* Enable XT1, disable X1 */
    CSC.csc = 0x80U;                                        /* Start XT1 and HOCO, stop X1 */
    CKC.ckc = 0x00U; 

    /* Delay 1 second */
    register unsigned long int i;
    for (i = 0x000FFFFFUL; i; --i)
        asm("nop");
    OSMC.osmc = 0x00U;                                      /* Supply Fil to Interval Timer. Fsub not present in 20-36 pin MCUs */

    /* Setup 12-bit interval timer */
    RTCEN = 1;                                              /* Enable 12-bit interval timer and RTC */
    ITMK = 1;                                               /* Disable IT interrupt */
    ITPR0 = 0;                                              /* Set interrupt priority - highest */
    ITPR1 = 0;
    ITMC.itmc = 0x8FFF;                                    /* Set maximum period ~270ms and start timer */
    ITIF = 1;                                               /* Set interrupt request flag */
    ITMK = 0;                                               /* Enable IT interrupt */
    asm ("ei");                                             /* Enable interrupts, immediate IT interrupt must happen */
}