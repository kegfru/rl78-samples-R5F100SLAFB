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
    LED1 ^= 1;
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
    InitialiseDisplay();
    DisplayString(LCD_LINE1, "абвгдежз");
    DisplayString(LCD_LINE2, "ИКЛМНОПР");

    for(;;)
    {
        if (flag_1hz)
        {
            LED2 = 0;
            flag_1hz = 0;
            LED2 = 1;
        }
        asm("halt");
    }
}
