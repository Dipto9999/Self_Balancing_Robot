#include "lcd.h"


void LCD_Init(lcd* lcd, TIM_HandleTypeDef* timer)
{
	lcd->timer = timer;

	HAL_GPIO_WritePin(LCD_EN_GPIO_Port, LCD_EN_Pin, GPIO_PIN_RESET);
	HAL_Delay(40);

	LCD_WriteCommand(lcd, 0x33);
	HAL_Delay(5);
	LCD_WriteCommand(lcd, 0x33);
	HAL_Delay(5);
	LCD_WriteCommand(lcd, 0x32);
	HAL_Delay(5);

	LCD_WriteCommand(lcd, 0x28);
	HAL_Delay(5);
	LCD_WriteCommand(lcd, 0x0c);
	HAL_Delay(5);
	LCD_ClearDisplay(lcd);


}



void LCD_StartTimer(lcd* lcd, uint8_t microseconds)
{
	lcd->timerflag = false;
	__HAL_TIM_SET_AUTORELOAD(lcd->timer, microseconds);
	HAL_TIM_Base_Start_IT(lcd->timer);
}

void LCD_SendByte(lcd* lcd, uint8_t byte)
{
	HAL_GPIO_WritePin(LCD_D7_GPIO_Port, LCD_D7_Pin, (byte >> 7) & 0x01);
	HAL_GPIO_WritePin(LCD_D6_GPIO_Port, LCD_D6_Pin, (byte >> 6) & 0x01);
	HAL_GPIO_WritePin(LCD_D5_GPIO_Port, LCD_D5_Pin, (byte >> 5) & 0x01);
	HAL_GPIO_WritePin(LCD_D4_GPIO_Port, LCD_D4_Pin, (byte >> 4) & 0x01);

	HAL_GPIO_WritePin(LCD_EN_GPIO_Port, LCD_EN_Pin, GPIO_PIN_SET);
	LCD_StartTimer(lcd, 40);
	while(!lcd->timerflag);
	HAL_GPIO_WritePin(LCD_EN_GPIO_Port, LCD_EN_Pin, GPIO_PIN_RESET);

	HAL_GPIO_WritePin(LCD_D7_GPIO_Port, LCD_D7_Pin, (byte >> 3) & 0x01);
	HAL_GPIO_WritePin(LCD_D6_GPIO_Port, LCD_D6_Pin, (byte >> 2) & 0x01);
	HAL_GPIO_WritePin(LCD_D5_GPIO_Port, LCD_D5_Pin, (byte >> 1) & 0x01);
	HAL_GPIO_WritePin(LCD_D4_GPIO_Port, LCD_D4_Pin, byte & 0x01);
	// Very Inefficient Way of Toggling Pins but may decide to use registers instead in the future

	HAL_GPIO_WritePin(LCD_EN_GPIO_Port, LCD_EN_Pin, GPIO_PIN_SET);
	LCD_StartTimer(lcd, 40);
	while(!lcd->timerflag);
	HAL_GPIO_WritePin(LCD_EN_GPIO_Port, LCD_EN_Pin, GPIO_PIN_RESET);

}

void LCD_WriteData(lcd* lcd, uint8_t data)
{
	HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_SET);
	LCD_SendByte(lcd, data);
}


void LCD_WriteCommand(lcd* lcd, uint8_t command)
{
	HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_RESET);
	LCD_SendByte(lcd, command);
}

void LCD_ClearDisplay(lcd* lcd)
{
	LCD_WriteCommand(lcd, 0x01);
	HAL_Delay(20);
}


void LCD_SetCursor(lcd* lcd, uint8_t row, uint8_t column) {

	uint8_t address = (row == 0) ? 0x00 : 0x40;
	address += column;
	LCD_WriteCommand(lcd, 0x80 | address);

}

void LCD_DisplayChar(lcd* lcd, uint8_t row, uint8_t column, uint8_t character)
{
	LCD_SetCursor(lcd, row, column);
	LCD_WriteData(lcd, character);
}

void LCD_DisplayString(lcd* lcd, uint8_t row, uint8_t column, char* string)
{
	for (uint8_t i = 0; i < strlen(string); i++)
	{
		LCD_SetCursor(lcd, row, column);
		LCD_WriteData(lcd, string[i]);
		column++;
		column %= 16;
	}


}





