#include "macro.h"
#include "rtc.h"
#include "lcd0.h"

/* Create a new instance of data structure RTCCounterValue */
struct RTCCounterValue stRTC_Data;

/* Declare a buffer to hold the time for displaying on the debug LCD */
int8_t lcd_buffer_l1[] = "00.00.00";
int8_t lcd_buffer_l2[] = "00:00:00";

void R_RTC_Callback_ConstPeriod(void)
{
	/* Read the RTC data */
	R_RTC_Get_CounterValue(&stRTC_Data);

	/* Read the seconds value */
	lcd_buffer_l2[7] = (int8_t)((stRTC_Data.Sec & 0x0F) + 0x30);
	lcd_buffer_l2[6] = (int8_t)((stRTC_Data.Sec >> 4) + 0x30);
	/* Read the minutes value */
	lcd_buffer_l2[4] = (int8_t)((stRTC_Data.Min & 0x0F) + 0x30);
	lcd_buffer_l2[3] = (int8_t)((stRTC_Data.Min >> 4) + 0x30);
	/* Read the hours value */
	lcd_buffer_l2[1] = (int8_t)((stRTC_Data.Hour & 0x0F) + 0x30);
	lcd_buffer_l2[0] = (int8_t)((stRTC_Data.Hour >> 4) + 0x30);

	lcd_buffer_l1[7] = (int8_t)((stRTC_Data.Year & 0x0F) + 0x30);
	lcd_buffer_l1[6] = (int8_t)((stRTC_Data.Year >> 4) + 0x30);

	lcd_buffer_l1[4] = (int8_t)((stRTC_Data.Month & 0x0F) + 0x30);
	lcd_buffer_l1[3] = (int8_t)((stRTC_Data.Month >> 4) + 0x30);

	lcd_buffer_l1[1] = (int8_t)((stRTC_Data.Day & 0x0F) + 0x30);
	lcd_buffer_l1[0] = (int8_t)((stRTC_Data.Day >> 4) + 0x30);

	/* Display the time on the debug LCD */
	DisplayString(LCD_LINE1, lcd_buffer_l1);
	DisplayString(LCD_LINE2, lcd_buffer_l2);
}


struct RTCCounterValue wrRTC_Data;

void R_RTC_Set_Time(char * date)
{
//	char date[]="26.03.2015 21:59:30";
  	wrRTC_Data.Sec |= (int8_t)((date[18] - 0x30) & 0x0F);
  	wrRTC_Data.Sec |= (int8_t)((date[17] - 0x30) << 4);

  	wrRTC_Data.Min |= (int8_t)((date[15] - 0x30) & 0x0F);
  	wrRTC_Data.Min |= (int8_t)((date[14] - 0x30) << 4);

  	wrRTC_Data.Hour |= (int8_t)((date[12] - 0x30) & 0x0F);
  	wrRTC_Data.Hour |= (int8_t)((date[11] - 0x30) << 4);

  	wrRTC_Data.Year |= (int8_t)((date[9] - 0x30) & 0x0F);
  	wrRTC_Data.Year |= (int8_t)((date[8] - 0x30) << 4);

  	wrRTC_Data.Month |= (int8_t)((date[4] - 0x30) & 0x0F);
  	wrRTC_Data.Month |= (int8_t)((date[3] - 0x30) << 4);

  	wrRTC_Data.Day |= (int8_t)((date[1] - 0x30) & 0x0F);
  	wrRTC_Data.Day |= (int8_t)((date[0] - 0x30) << 4);

	R_RTC_Set_CounterValue(wrRTC_Data);
}
