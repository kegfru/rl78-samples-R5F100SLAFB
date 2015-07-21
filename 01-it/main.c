#include "QB-R5F100SL-TB.h"

__attribute__((interrupt))
void wdti_handler(void)
{
}

__attribute__((interrupt))
void it_handler(void)
{
    LED1 ^= 1;
    LED2 ^= 1;
}

void main(void)
{
    /* Setup LEDs */
    LED1 = 1;
    LED2 = 0;
    LED1_PIN = 0;
    LED2_PIN = 0;

	main_init();	

    for(;;)
    {
        asm("stop");                                        /* Enter STOP mode, only interval timer stay running */
    }
}
