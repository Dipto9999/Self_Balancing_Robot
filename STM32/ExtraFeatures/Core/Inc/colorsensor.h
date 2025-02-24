#ifndef INC_COLORSENSOR_H_
#define INC_COLORSENSOR_H_

#include <stdbool.h>
#include "main.h"

typedef enum
{
	RED,
	GREEN,
	BLUE
} color;

typedef struct
{
    I2C_HandleTypeDef* i2c;
    uint8_t slave_address;
    uint16_t rgb_data[3];  // Store Red, Green, Blue values
    uint8_t i2c_commands[3];  // Commands for RGB
    uint8_t current_channel;

    bool enabled;
} colorsensor;

void ColorSensor_Init(colorsensor* sensor, I2C_HandleTypeDef* i2c_handle, uint8_t slave_addr);
void ColorSensor_EnableStatus(colorsensor* sensor, bool enable);
void ColorSensor_RGBStartColorReceive(colorsensor* sensor);
void ColorSensor_ReceiveInterrupt(colorsensor* sensor);
color ColorSensor_CalculateColor(colorsensor* sensor);
void ColorSensor_Handle(colorsensor* sensor);

#endif


