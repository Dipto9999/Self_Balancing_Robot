#ifndef INC_LCD_H_
#define INC_LCD_H_

#include "main.h"
#include <string.h>
#include <stdbool.h>

typedef struct {
	volatile bool timerflag;
	TIM_HandleTypeDef* timer;
} lcd;


void LCD_Init(lcd* lcd, TIM_HandleTypeDef* timer);
void LCD_StartTimer(lcd* lcd, uint8_t microseconds);
void LCD_SendByte(lcd* lcd, uint8_t byte);
void LCD_WriteData(lcd* lcd, uint8_t data);
void LCD_WriteCommand(lcd* lcd, uint8_t command);
void LCD_ClearDisplay(lcd* lcd);
void LCD_SetCursor(lcd* lcd, uint8_t row, uint8_t column);
void LCD_DisplayChar(lcd* lcd, uint8_t row, uint8_t column, uint8_t character);
void LCD_DisplayString(lcd* lcd, uint8_t row, uint8_t column, char* string);



#endif /* INC_LCD_H_ */
