/*
 * lcd.c
 *
 *  Created on: Feb 20, 2025
 *      Author: User123
 */


/* Private includes -----------------------------------------------*/
#include "lcd.h"

/* Private defines ------------------------------------------------*/

/* Private macros -------------------------------------------------*/

/* Private typedefs ------------------------------------------------*/

/* Private constants -----------------------------------------------*/
const uint8_t ucLcdRowOffset[4] = {0x00, 0x40, 0x14, 0x54};

/* Private variables -----------------------------------------------*/
//static uint8_t ucLcdIsInit = 0x00;

/* Private prototype function --------------------------------------*/
//static uint8_t LCD_Wait_Busy(void);
void LCD_WriteNibble(uint8_t data, uint8_t isCmd);

/* Exported reference function -------------------------------------*/
stm32_err_t LCD_Init(lcd_interface_t eInterface)
{
	/* Configure GPIOs */
	LCD_Port->CRL = 0x22222222;
	LCD_Port->CRH = 0x00000222;
	LCD_Port->ODR &= ~LCD_DB_Mask;
	LCD_Port->ODR &= ~LCD_CTRL_Mask;

	//AFIO_PCLK_EN();
	/* Enable AFIO Clock */
	//__HAL_RCC_AFIO_CLK_ENABLE();
	//(RCC->APB2ENR |= (1 << 0));
	//AFIO -> MAPR |= 0x02000000;			// GPIO PB4, PB3 and PA15 will be free


	/** Perform init sequence: see pag. 45-46 on
	 * @link https://cdn.sparkfun.com/assets/9/5/f/7/b/HD44780.pdf @endlink
	 */


	if(eInterface == LCD_4BITS_INTERFACE)
	{
		// Power-on delay
		delay_ms(40);

		// Set 8-bit interface
		//LCD_WriteNibble(LCD_CMD_FUNCTION_SET | LCD_4B_INTERFACE, LCD_COMMAND);
		LCD_Write(0x03, LCD_COMMAND);

		// Delay
		delay_ms(5);

		// Resend
		//LCD_WriteNibble(LCD_CMD_FUNCTION_SET | LCD_4B_INTERFACE, LCD_COMMAND);
		LCD_Write(0x03, LCD_COMMAND);

		// 100 us delay
		delay_us(150);

		// Resend
		//LCD_WriteNibble(LCD_CMD_FUNCTION_SET | LCD_4B_INTERFACE, LCD_COMMAND);
		LCD_Write(0x03, LCD_COMMAND);

		LCD_Write(0x02, LCD_COMMAND);

		// 100 us delay
		//delay_us(100);

		// The LCD.BUSY_FLAG can now be set by the LCD
		//ucLcdIsInit = 1;

		// Configure LCD settings
		//LCD_WriteNibble(LCD_CMD_FUNCTION_SET | LCD_4B_INTERFACE, LCD_COMMAND);
		LCD_Write(0x28, LCD_COMMAND);

		LCD_Write(0x0C, LCD_COMMAND);

		LCD_Write(0x01, LCD_COMMAND);

		LCD_Write(0x0C, LCD_COMMAND);

		// Delay
		//delay_ms(5);
		delay_ms(2);

		LCD_Write(0x06, LCD_COMMAND);

		LCD_Write(0x80, LCD_COMMAND);



		/*
		LCD_WriteNibble(LCD_CMD_FUNCTION_SET | LCD_4B_INTERFACE, LCD_COMMAND);

		LCD_WriteNibble(LCD_4B_TWO_LINES | LCD_5X8_FONT, LCD_COMMAND);

		LCD_WriteNibble(0x00, LCD_COMMAND);

		LCD_WriteNibble(LCD_4B_TWO_LINES | LCD_5X8_FONT, LCD_COMMAND);

		LCD_WriteNibble(0x00, LCD_COMMAND);

		LCD_WriteNibble(LCD_4B_CMD_CLEAR, LCD_COMMAND);

		LCD_WriteNibble(0x00, LCD_COMMAND);

		LCD_WriteNibble(LCD_4B_CMD_ENTRY_MODE | LCD_4B_CURSOR_INC, LCD_COMMAND);


		// Display: ON
		// Cursor: ON
		// Cursor.Blink: OFF
		LCD_WriteNibble(0x00, LCD_COMMAND);

		LCD_WriteNibble(LCD_4B_CMD_DISPLAY_CONTROL | LCD_4B_DISPLAY_ON, LCD_COMMAND);

		// Display clear
		//LCD_Write(LCD_CMD_CLEAR, LCD_COMMAND);

		// Entry mode
		// Address: Increment
		// Display: No Shift
		//LCD_Write(LCD_CMD_ENTRY_MODE | LCD_CURSOR_INC, LCD_COMMAND);
		 */
	}
	else if(eInterface == LCD_8BITS_INTERFACE)
	{
		// 4-bit interface: see pag. 22, and 46

	}

	return STM32_OK;
}

void LCD_Write(uint8_t data, uint8_t isCmd)
{
	LCD_WriteNibble(data, isCmd);
	LCD_WriteNibble(data<<4, isCmd);

}

// Write sequence
void LCD_WriteNibble(uint8_t data, uint8_t isCmd)
{
	//if(ucLcdIsInit) LCD_Wait_Busy();
	//delay_ms(2);

	// Write mode (RW = 0)
	LCD_Port->BRR = LCD_RW_Pin;

	// Configure DB pins as outputs
	//LCD_Port->CRL = 0x22222222; Ya configuramos esto antes

	// Write data/instruction (RS = 1/0)
	if( isCmd == LCD_COMMAND ){
		LCD_Port->BRR = LCD_RS_Pin;
	}
	else{
		LCD_Port->BSRR = LCD_RS_Pin;
	}

	// Write data into DB pins
	LCD_Port->BRR = LCD_DB_Mask;
	LCD_Port->BSRR |= (uint32_t)data<<1;

	// Send EN pulse: see Bus Timing Characteristics on pag. 49
	LCD_Port->BSRR = LCD_E_Pin; // E = 1
	delay_ms(1);

	// 1us delay assuming an eight MHz clock needs 8 clock cycles
	//delay_us(1);
	LCD_Port->BRR = LCD_E_Pin;	// E=0
	delay_us(2);

	// Set DB pins to high
	LCD_Port->BSRR = LCD_DB_Mask;
}

// Read sequence
uint8_t LCD_Read(uint8_t isData)
{
	uint8_t dout = 0x00;

	// Read mode (RW = 1)
	LCD_Port->BSRR = LCD_RW_Pin;

	// Read data/busy&DDRAM (RS = 1/0)
	if(isData) LCD_Port->BSRR = LCD_RS_Pin;
	else LCD_Port->BRR = LCD_RS_Pin;

	// Configure DB pins as floating inputs
	LCD_Port->CRL = 0x44444444;

	// Execution cycle: see Bus Timing Characteristics on pag. 49
	LCD_Port->BSRR = LCD_E_Pin; // E = 1

	// 1us delay assuming an eight MHz clock needs 8 clock cycles
	delay_us(1);

	// Read data from DB pins
	dout = (uint8_t)(LCD_Port->IDR & 0xFF);

	// E = 0
	LCD_Port->BRR = LCD_E_Pin;

	// 1us delay assuming an eight MHz clock needs 8 clock cycles
	delay_us(1);

	return dout;
}

/**
  * @brief  Set LCD Address Counter (AC).
  * @param  x The column position.
  * 		 This parameter ranging from 0 to LCD_NUM_COLS-1.
  * @param  y The row position.
  * 		 This parameter ranging from 0 to LCD_NUM_ROWS-1.
  * @retval STM32 error status.
  * 		 The return value can be one of the following:
  * 		   @arg STM32_OK: If everything is OK.
  * 		   @arg STM32_ERR_OUT_RANGE: If x>=LCD_NUM_COLS or y>=LCD_NUM_ROWS.
  */
stm32_err_t LCD_Goto_XY(uint8_t x, uint8_t y)
{
	if(x > LCD_NUM_COLS-1 || y > LCD_NUM_ROWS-1) return STM32_ERR_OUT_RANGE;

	LCD_Write(LCD_CMD_SET_DDRAM_ADDR | ucLcdRowOffset[y] | x, LCD_COMMAND);

	return STM32_OK;
}

stm32_err_t LCD_Print(const char *pcString)
{
	// Assert pointer
	if(!pcString) return STM32_ERR_NULL_POINTER;

	// Write string
	while(*pcString)
	{
		LCD_Write(*pcString++, LCD_DATA);
	}

	return STM32_OK;
}

stm32_err_t LCD_ScrollText(lcd_shift_t dir, uint8_t steps, uint32_t speed)
{
	// Scroll
	for(int i = 0; i < steps; i++)
	{
		LCD_Write(LCD_CMD_SHIFT | LCD_SHIFT_DISPLAY | (uint8_t)dir*LCD_RIGHT_SHIFT, LCD_COMMAND);
		delay_ms(speed);
	}

	return STM32_OK;
}

stm32_err_t LCD_Create5X8Char(uint8_t loc, const uint8_t *charmap)
{
	// Assert argin
	if(!charmap) return STM32_ERR_NULL_POINTER;
	if( (loc+1) << 3 > LCD_GCRAM_DEPTH - 1) return STM32_ERR_OUT_RANGE;

	// Set CG address
	LCD_Write(LCD_CMD_SET_CGRAM_ADDR | loc << 3, LCD_COMMAND);

	// Write the 5x8 (WIDTH x HEIGHT) custom char
	for(int i = 0; i < 8; i++)
	{
		LCD_Write(charmap[i], LCD_DATA);
	}

	// The new custom char can now be printed by writing
	// the location of this char (0x00 - 0x07)
	return STM32_OK;
}
/*
 * @name	int2str
 * @brief	This function formats an int32 variable into a string
 *
 * @arg 	x, int32_t, variable to convert.
 * @arg		pcBuffer, char *, a char array of at most 10 items where the
 * 			string will be stored
 * @arg		pre, uint8_t. the precision of the conversion
 *
 * @return	The output string length
 */
uint8_t int2str(uint32_t x, char *pcBuffer)
{
	uint8_t ndigs = 0;
	do {
		pcBuffer[ndigs++] = '0' + (x % 10); // Extract least significant digit
		x /= 10;
	} while (x > 0);
    pcBuffer[ndigs--] = '\0'; // Set the end string character


	// Reverse the string into buffer
    uint8_t ndigstmp = ndigs;
	uint8_t j = 0;
	while (ndigstmp > j) {
		char tmp = pcBuffer[j];
		pcBuffer[j++] = pcBuffer[ndigstmp];
		pcBuffer[ndigstmp--] = tmp;
	}

	return ndigs+1;
}

/*
 * @name	float2str
 * @brief	This function formats a float variable into a string.
 *
 * @arg 	x, float, variable to convert
 * @arg		pcBuffer, char *, a char array of at most 13 items where
 * 			the string will be stored.
 * @arg		pre, uint8_t. the precision of the conversion.
 *
 * @return	The output string length
 */
uint8_t float2str(float x, char *pcBuffer, uint8_t pre)
{
	// Split the integer and decimal part
	int16_t integer = (int16_t)x;
	float fDec = (x - integer);
	for(uint8_t i = 0; i < pre; i++)
	{
		fDec *= 10.0;
	}
	uint16_t dec = (int16_t)fDec;

	// Convert integer part to char
	uint8_t isNegative = 0;
	if(integer < 0)
	{
		integer = -integer;
		isNegative = 1;
		pcBuffer[0]='-';
	}
	uint8_t ndigs = int2str((uint32_t)integer, &pcBuffer[isNegative]);

	// Convert decimal part to char
	pcBuffer[isNegative+ndigs] = '.';
	uint8_t ndigsdec = int2str((uint32_t)dec, &pcBuffer[isNegative+ndigs+1]);
	pcBuffer[ndigs + isNegative + ndigsdec + 1] = '\0';

	return ndigs + isNegative + ndigsdec + 1;
}


/* Private reference function --------------------------------------*/
/*
static uint8_t LCD_Wait_Busy(void)
{
	// Wait until busy flag is cleared
	while (LCD_Read(0) & LCD_BUSY);

	// Delay needed for address counter is updated after busy flag is cleared
	delay_us(10);

	// Read and return address counter
	return LCD_Read(0);
}


*/


