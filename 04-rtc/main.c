#include "QB-R5F100SL-TB.h"
#include "lcd0.h"

volatile unsigned char ticks = 0;
volatile unsigned char flag_1hz = 0;

__attribute__((interrupt))
void wdti_handler(void)
{
}

__attribute__((interrupt))
void it_handler(void)
{
    ++ticks;
    if (0 == (0x07 & ticks))
    {
        flag_1hz = 1;
    }
}

int main(void)
{
    /* Setup LEDs */
    LED1 = 1;
    LED2 = 1;
    LED1_PIN = 0;
    LED2_PIN = 0;

	main_init();
    /* Enable RTC signal output */
	RCLOE1 = 1;
	/* Enable the Real Time Clock */
	R_RTC_Create();
	R_RTC_Start();

	R_RTC_Set_Time("21.03.2017 11:01:00");

    InitialiseDisplay();

    for(;;)
    {
        if (flag_1hz)
        {
			LED2 ^= 1;
            flag_1hz = 0;
			R_RTC_Callback_ConstPeriod();
        }
        asm("halt");
    }
}
