#include "lcd0.h"
#include <iodefine.h>

unsigned  char  ROM1602Cyr_table[]=
{
  0x41,0xa0,0x42,0xa1,0xe0,0x45,0xa3,0xa4,
  0xa5,0xa6,0x4b,0xa7,0x4d,0x48,0x4f,0xa8,
  0x50,0x43,0x54,0xa9,0xaa,0x58,0xe1,0xab,
  0xac,0xe2,0xad,0xae,0x62,0xaf,0xb0,0xb1,
  0x61,0xb2,0xb3,0xb4,0xe3,0x65,0xb6,0xb7,
  0xb8,0xb9,0xba,0xbb,0xbc,0xbd,0x6f,0xbe,
  0x70,0x63,0xbf,0x79,0xe4,0x78,0xe5,0xc0,
  0xc1,0xe6,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,
};

void InitialiseDisplay(void)
{	

	// Set pins as output
    LCD_RS_PIN = 0;
    LCD_EN_PIN = 0;    
    LCD_D4_PIN = 0;
    LCD_D5_PIN = 0;    
    LCD_D6_PIN = 0;
    LCD_D7_PIN = 0; 

	LCD_EN = SET_BIT_HIGH;
	DisplayDelay(7000); 
	LCD_EN = SET_BIT_LOW;
  
	/* Display initialises in 8 bit mode - so send one write (seen as 8 bit) to set to 4 bit mode. */
	/* Function Set */
	LCD_nibble_write(CTRL_WR,0x03);
	LCD_nibble_write(CTRL_WR,0x03);
	DisplayDelay(39);

	/* Configure display */
	LCD_nibble_write(CTRL_WR,0x03);
	LCD_nibble_write(CTRL_WR,0x02);
	LCD_nibble_write(CTRL_WR,(LCD_DISPLAY_ON | LCD_TWO_LINE ));
	LCD_nibble_write(CTRL_WR,(LCD_DISPLAY_ON | LCD_TWO_LINE ));
	DisplayDelay(39);
	
	/* Display ON/OFF control */
	LCD_write(CTRL_WR,LCD_CURSOR_OFF);
	DisplayDelay(39);

	/* Display Clear */
	LCD_write(CTRL_WR,LCD_CLEAR);
	DisplayDelay(1530);

	/* Entry Mode Set */
	LCD_write(CTRL_WR,0x06);
	LCD_write(CTRL_WR,LCD_HOME_L1);
	
}

void DisplayString(unsigned char position, char * string)
{
	static unsigned char next_pos = 0xFF;

	/* Set line position if needed. We don't want to if we don't need to because LCD control operations take longer than LCD data operations. */
	if( next_pos != position)
	{
		if(position == LCD_LINE1)
		{
			/* Display on Line 1 */
		  	LCD_write(CTRL_WR, ((char)(LCD_HOME_L1 + position)));
		}
		if(position == LCD_LINE2)
		{
			/* Display on Line 2 */
		  	LCD_write(CTRL_WR, ((char)((LCD_HOME_L2 + position) - LCD_LINE2)));
		}
		if(position == LCD_LINE3)
		{
			/* Display on Line 3 */
		  	LCD_write(CTRL_WR, ((char)((LCD_HOME_L3 + position) - LCD_LINE3)));
		}
		if(position == LCD_LINE4)
		{
			/* Display on Line 4 */
		  	LCD_write(CTRL_WR, ((char)((LCD_HOME_L4 + position) - LCD_LINE4)));
		}
		/* set position index to known value */
		next_pos = position;		
	}

	do
	{
		char shift=0;
  		char outchar=0;
  		char grad_sign = 0;
// http://developer.mbed.org/users/margadon/code/Freetronics_16x2_LCD_Rus/file/08abb319acab/freetronicsLCDShieldRus.cpp
		if (*string>=0x80) {
			if (*string==0xd0)shift=0x90;
			if (*string==0xd1)shift=0x50;
            if (*string==0xc2)grad_sign = 1;
			else grad_sign = 0;
			*string++;
			outchar=ROM1602Cyr_table[*string-shift];
            if (*string==0x91&shift==0x50)outchar=0xb5; //ё  
            if (*string==0x81&shift==0x90)outchar=0xa2; //Ё  
            if (*string==0xb0&grad_sign==1)outchar=0xef; //° Alt+0176
			LCD_write(DATA_WR,outchar);
			*string++;
		}
		else {
			LCD_write(DATA_WR,*string++);
		}
		/* increment position index */
		next_pos++;				
	} 
	while(*string);
}

void DisplayStringPos(unsigned char position, char * string)
{
	LCD_write(CTRL_WR, ((char)(LCD_HOME_L1 + position)));
	do
	{
		LCD_write(DATA_WR,*string++);		
	} 
	while(*string);
}

void LCD_write(unsigned char data_or_ctrl, char value)
{
	/* Write upper nibble first */
	LCD_nibble_write(data_or_ctrl, (char)((value & 0xF0) >> 4));
	
	/* Write lower nibble second */
	LCD_nibble_write(data_or_ctrl, (char)(value & 0x0F));
}

void LCD_nibble_write(unsigned char data_or_ctrl, char value)
{
	char ucStore;
	
	/* Set Register Select pin high for Data */
	if (data_or_ctrl == DATA_WR)
	{
		LCD_RS = SET_BIT_HIGH;
	}
	else
	{
		LCD_RS = SET_BIT_LOW;
	}
	
	/* There must be 40ns between RS write and EN write */
  	DisplayDelay(1);					
	
  	/* EN enable chip (HIGH) */
	LCD_EN = SET_BIT_HIGH;
	
	/* Tiny delay */  			
  	DisplayDelay(1);
  	
    /* Clear port bits used */	
	ucStore = LCD_DATA_PORT;
	ucStore &= (char) ~DATA_PORT_MASK;	
	
	/* OR in data */	
	ucStore |= (char) ((value << DATA_PORT_SHIFT) & DATA_PORT_MASK );	

	/* Write data to port */	
	LCD_DATA_PORT = ucStore;
	
	/* write delay while En High */	            
	DisplayDelay(20);
	
	/* Latch data by dropping EN */					
    	LCD_EN = SET_BIT_LOW;
	
	/* Data hold delay */				
	DisplayDelay(20);					
	
	if (data_or_ctrl == CTRL_WR)
	{
		/* Extra delay needed for control writes */
		DisplayDelay(0x7FF);
	}
}

void DisplayDelay(unsigned int units)
{
	unsigned int counter = units * DELAY_TIMING;
	
	while(counter--)
	{
		/* Delay Loop	*/
		asm("nop");
	}
}

