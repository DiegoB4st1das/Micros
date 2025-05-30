/*
 * lcd.h
 *
 *  Created on: Feb 20, 2025
 *  @Author: User123
 *  @Notes: https://cdn.sparkfun.com/assets/9/5/f/7/b/HD44780.pdf
 *    CGRAM: Character generator RAM. It is a 64-bytes RAM memory,
 *    		 where custom 5x8 characters can be programmed.
 *    DDRAM: Data DRAM. It is a
 *    AC: Address Counter
 */

#ifndef LCD_H_
#define LCD_H_

/* Exported includes -----------------------------------------------*/
#include "stm32f103xb.h"
#include "common.h"

/* Exported defines ------------------------------------------------*/
// Pinout
#define LCD_Port		GPIOB
#define LCD_DB_Mask		(0x01E0)
#define LCD_RS_Pin 		BIT(3)
#define LCD_RW_Pin 		BIT(2)
#define LCD_E_Pin		BIT(4)
#define LCD_CTRL_Mask	(LCD_E_Pin | LCD_RS_Pin | LCD_RW_Pin)

// LCD
#define LCD_MAX_COLS	40
#define LCD_NUM_ROWS	2
#define LCD_NUM_COLS	16
#define LCD_GCRAM_DEPTH	64
#define LCD_DDRAM_DEPTH	80

// Flags
#define LCD_COMMAND 0x01
#define LCD_DATA	0x00

// LCD Commands: see pag. 24
#define LCD_CMD_CLEAR		0x01
#define LCD_4B_CMD_CLEAR	0x10

#define LCD_CMD_RETURN_HOME 0x02

// I/D, S
#define LCD_CMD_ENTRY_MODE		0x04
#define LCD_4B_CMD_ENTRY_MODE	0x40
#define LCD_DISPLAY_SHIFT		0x01		// Shift display to right
#define LCD_DISPLAY_NO_SHIFT	0x00		// Display no shift
#define LCD_CURSOR_INC 			0x02		// Cursor increment
#define LCD_4B_CURSOR_INC 		0x20
#define LCD_CURSOR_DEC 			0x00		// Cursor decrement

// D, C, B
#define LCD_CMD_DISPLAY_CONTROL 0x08
#define LCD_4B_CMD_DISPLAY_CONTROL 0x80
#define LCD_DISPLAY_ON			0x04		// Turn on display
#define LCD_4B_DISPLAY_ON		0x40
#define LCD_DISPLAY_OFF			0x00		// Turn off display
#define LCD_CURSOR_ON			0x02		// Turn on cursor
#define LCD_CURSOR_OFF			0x00		// Turn off cursor
#define LCD_CURSOR_BLINK 		0x01		// Turn on blink cursor
#define LCD_CURSOR_NO_BLINK 	0x00		// Turn off blink cursor

// S/C, R/L: Moves cursor and shifts display without changing DDRAM contents
#define LCD_CMD_SHIFT 			0x10
#define LCD_SHIFT_DISPLAY 		0x08	// Shift display one position
#define LCD_CURSOR_MOVE 		0x00	// Just move the cursor one position
#define LCD_RIGHT_SHIFT 		0x04	// Shift/move display/cursor to the right
#define LCD_LEFT_SHIFT 			0x00	// Shift/move display/cursor to the left

// DL, N, F: see pag. 10-11
#define LCD_CMD_FUNCTION_SET 	0x20
#define LCD_8B_INTERFACE 		0x10		// Set interface data length
#define LCD_4B_INTERFACE 		0x00		// Set interface data length
#define LCD_TWO_LINES 			0x08
#define LCD_4B_TWO_LINES 		0x80        // Set number of display lines
#define LCD_ONE_LINE 			0x00		// Set number of display lines
#define LCD_5X10_FONT 			0x04		// Set font size
#define LCD_5X8_FONT 			0x00		// Set font size

#define LCD_CMD_SET_CGRAM_ADDR 	0x40		// Set CGRAM (Character Generator) address
#define LCD_CGRAM_ADDR_MASK		0x3F

#define LCD_CMD_SET_DDRAM_ADDR 	0x80		// Set DDRAM (Data) address
#define LCD_DDRAM_ADDR_MASK		0x7F

#define LCD_CMD_READ_BUSY_ADDR	0x00
#define LCD_BUSY_MASK			0x80
#define LCD_BUSY				0x80
#define LCD_IDLE				0x00
#define LCD_ADDR_COUNTER_MASK	0x7F

/* Exported macros -------------------------------------------------*/

/* Exported typedefs ------------------------------------------------*/
typedef enum
{
	LCD_4BITS_INTERFACE=0x00,
	LCD_8BITS_INTERFACE=0x10
} lcd_interface_t;

typedef enum
{
	LCD_LEFT_DIRECTION = 0,
	LCD_RIGHT_DIRECTION
} lcd_shift_t;

/* Exported variables -----------------------------------------------*/

/* Exported prototype function --------------------------------------*/
// Basic LCD I/O driver
// See pag. 32-33, 45-46
stm32_err_t LCD_Init(lcd_interface_t eInterface);

// See pag. 24-25
void LCD_Write(uint8_t data, uint8_t isCmd);

// See pag. 24-25
uint8_t LCD_Read(uint8_t isData);

// Extended
uint8_t int2str(uint32_t x, char *pcBuffer);
uint8_t float2str(float x, char *pcBuffer, uint8_t pre);

stm32_err_t LCD_Goto_XY(uint8_t x, uint8_t y);
stm32_err_t LCD_Print(const char *pcString);
stm32_err_t LCD_ScrollText(lcd_shift_t dir, uint8_t steps, uint32_t speed);
stm32_err_t LCD_Create5X8Char(uint8_t loc, const uint8_t *charmap);


#endif /* LCD_H_ */
