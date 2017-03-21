#include "rtc.h"
#include "lcd0.h"

void R_RTC_Create(void)
{
	//RTCEN = 1U;		/* supply RTC clock */
	RTCE = 0U;		/* disable RTC clock operation */
	RTCMK = 1U;		/* disable INTRTC interrupt */
	RTCIF = 0U;		/* clear INTRTC interrupt flag */
	/* Set INTRTC level 1 priority */
	RTCPR1 = 0U;
	RTCPR0 = 1U;
	RTCC0.rtcc0 = _00_RTC_RTC1HZ_DISABLE | _08_RTC_24HOUR_SYSTEM | _02_RTC_INTRTC_CLOCK_1;
}

void R_RTC_Start(void)
{
	RTCIF = 0U;	/* clear INTRTC interrupt flag */
	RTCMK = 0U;	/* enable INTRTC interrupt */
	RTCE = 1U;	/* enable RTC clock operation */
}

MD_STATUS R_RTC_Get_CounterValue(struct RTCCounterValue *counterreadval)
{
	MD_STATUS status = MD_OK;
	uint32_t i;
	
	RTCC1.rtcc1 |= _01_RTC_COUNTER_PAUSE;
	/* Wait 10 us */
	for( i=0U; i<RTC_WAITTIME; i++ )
	{
		asm("nop");
	}
	if (RWST == 0U)
	{
		status = MD_BUSY1;
	}
	else
	{
		counterreadval->Sec = SEC.sec;
		counterreadval->Min = MIN.min;
		counterreadval->Hour = HOUR.hour;
		counterreadval->Week = WEEK.week;
		counterreadval->Day = DAY.day;
		counterreadval->Month = MONTH.month;
		counterreadval->Year = YEAR.year;
		RTCC1.rtcc1 &= (uint8_t)~_01_RTC_COUNTER_PAUSE;
		/* Wait 10 us */
		for( i=0U; i<RTC_WAITTIME; i++ )
		{
			asm("nop");
		}
		if (RWST == 1U)
		{
			status = MD_BUSY2;
		}
	}

	return (status);
}

MD_STATUS R_RTC_Set_CounterValue(struct RTCCounterValue counterwriteval)
{
	MD_STATUS status = MD_OK;
	uint32_t i;
	
	RTCC1.rtcc1 |= _01_RTC_COUNTER_PAUSE;
	/* Wait 10 us */
	for( i=0U; i<RTC_WAITTIME; i++ )
	{
		asm("nop");
	}
	if (RWST == 0U)
	{
		status = MD_BUSY1;
	}
	else
	{
		SEC.sec = counterwriteval.Sec;
		MIN.min = counterwriteval.Min;
		HOUR.hour = counterwriteval.Hour;
		WEEK.week = counterwriteval.Week;
		DAY.day = counterwriteval.Day;
		MONTH.month = counterwriteval.Month;
		YEAR.year = counterwriteval.Year;
		RTCC1.rtcc1 &= (uint8_t)~_01_RTC_COUNTER_PAUSE;
		/* Wait 10 us */
		for( i=0U; i<RTC_WAITTIME; i++ )
		{
			asm("nop");
		}
		if (RWST == 1U)
		{
			status = MD_BUSY2;
		}
	}

	return (status);
}

MD_STATUS R_RTC_Set_ConstPeriodInterruptOn(enum RTCINTPeriod period)
{
	MD_STATUS status = MD_OK;

	if ((period < HALFSEC) || (period > ONEMONTH))
	{
		status = MD_ARGERROR;
	}
	else
	{
		RTCMK = 1U;	/* disable INTRTC */
		RTCC0.rtcc0 = (RTCC0.rtcc0 & _88_RTC_INTRTC_CLEAR) | period;
		RTCC1.rtcc1 &= (uint8_t)~_08_RTC_INTC_GENERATE_FLAG;
		RTCIF = 0U;	/* clear INTRTC interrupt flag */
		RTCMK = 0U;	/* enable INTRTC interrupt */
	}

	return (status);
}

void R_RTC_Set_ConstPeriodInterruptOff(void)
{
	RTCC0.rtcc0 &= _88_RTC_INTRTC_CLEAR;
	RTCIF = 0U;		/* clear INTRTC interrupt flag */
}
