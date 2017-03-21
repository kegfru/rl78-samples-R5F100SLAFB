#include <iodefine.h>

#ifndef LCD0_H__
#define LCD0_h__

/******************************************************************************
LCD Macro Defines
*******************************************************************************/
/* RS Register Select pin */
#define LCD_RS			P0.BIT.bit5
#define LCD_RS_PIN		PM0.BIT.bit5
/* Display Enable pin */	
#define LCD_EN			P0.BIT.bit6	
#define LCD_EN_PIN		PM0.BIT.bit6
/* Data pins */
#define LCD_D4			P7.BIT.bit0	
#define LCD_D5			P7.BIT.bit1	
#define LCD_D6			P7.BIT.bit2	
#define LCD_D7			P7.BIT.bit3	

#define LCD_DATA_PORT		P7.p7

/* Bit mask from entire port */	 	
#define DATA_PORT_MASK		0x0F
/* Number of bits data needs to shift */	
#define DATA_PORT_SHIFT		0

#define LCD_D4_PIN		PM7.BIT.bit0
#define LCD_D5_PIN		PM7.BIT.bit1
#define LCD_D6_PIN		PM7.BIT.bit2
#define LCD_D7_PIN		PM7.BIT.bit3

#define DATA_WR 		1
#define CTRL_WR 		0

#define SET_BIT_HIGH		1
#define SET_BIT_LOW		0

/* Set to ensure base delay */
#define DELAY_TIMING		0x04

/* number of lines on the LCD display */
#define NUMB_CHARS_PER_LINE	20
/* Maximum characters per line of LCD display. */	
#define MAXIMUM_LINES		4		

#define LCD_LINE1		0
#define LCD_LINE2		16
#define LCD_LINE3		8
#define LCD_LINE4		24

/* Clear LCD display and home cursor */
#define LCD_CLEAR		0x01
/* move cursor to line 1 */
#define LCD_HOME_L1		0x80
/* move cursor to line 2 */      
#define LCD_HOME_L2		0xC0
/* move cursor to line 3 */      
#define LCD_HOME_L3		0x94
/* move cursor to line 4 */      
#define LCD_HOME_L4		0xD4
/* Cursor auto decrement after R/W */  
#define CURSOR_MODE_DEC		0x04
/* Cursor auto increment after R/W */
#define CURSOR_MODE_INC		0x06
/* Setup, 4 bits,2 lines, 5X7 */
#define FUNCTION_SET		0x28
/* Display ON with Cursor */
#define LCD_CURSOR_ON		0x0E
/* Display ON with Cursor off */
#define LCD_CURSOR_OFF		0x0C
/* Display on with blinking cursor */
#define LCD_CURSOR_BLINK	0x0D
/*Move Cursor Left One Position */
#define LCD_CURSOR_LEFT		0x10
/* Move Cursor Right One Position */
#define LCD_CURSOR_RIGHT	0x14     
/* Turn on the display */
#define LCD_DISPLAY_ON		0x04
/* Set cursor to start of line 2 */
#define LCD_TWO_LINE		0x08

/* LCD intialisation function prototype */
void InitialiseDisplay(void);
/* LCD string display function prototype */
void DisplayString(unsigned char position, char * string);
/* LCD byte write function prototype */
void LCD_write(unsigned char data_or_ctrl, char value);
/* LCD nibble write function prototype */
void LCD_nibble_write(unsigned char data_or_ctrl, char value);
/* Delay function prototype */
void DisplayDelay(unsigned int units);

#endif // LCD0_H__
