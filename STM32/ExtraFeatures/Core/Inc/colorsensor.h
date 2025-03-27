#ifndef INC_COLORSENSOR_H_
#define INC_COLORSENSOR_H_

#include <stdbool.h>
#include "main.h"
#include "speaker.h"

#define TCS3472_SLAVE_ADDRESS 0x29
#define TCS3472_EXPECTED_ID 0x44

#define TCS3472_ID_REG 0x12

#define TCS3472_WTIME_REG 0x03
#define TCS3472_ATIME_REG 0xF6
#define TCS3472_ENABLE_REG 0x00

#define TCS3472_CDATAL_REG 0x14
#define TCS3472_RDATAL_REG 0x16
#define TCS3472_GDATAL_REG 0x18
#define TCS3472_BDATAL_REG 0x1A

typedef enum
{
	CLEAR,
	RED,
	GREEN,
	BLUE
} color;

typedef struct
{
    I2C_HandleTypeDef* i2c;
    uint8_t slave_address;
    uint16_t rgb_data[4];  // Store Red, Green, Blue values

    bool enabled;
} colorsensor;

void ColorSensor_Init(colorsensor* sensor, I2C_HandleTypeDef* i2c_handle);
void ColorSensor_EnableStatus(colorsensor* sensor, bool enable);
uint16_t ColorSensor_Read16(colorsensor* sensor, uint8_t reg);
void ColorSensor_ReadAll(colorsensor* sensor);
color ColorSensor_CalculateColor(colorsensor* sensor);
void ColorSensor_Handle(colorsensor* sensor);

#endif


