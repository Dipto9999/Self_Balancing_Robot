#include "colorsensor.h"

void ColorSensor_Init(colorsensor* sensor, I2C_HandleTypeDef* i2c_handle, uint8_t slave_addr) {
    sensor->i2c = i2c_handle;
    sensor->slave_address = slave_addr;

    // Manually initialize arrays since they cannot be initialized in the struct
    sensor->i2c_commands[0] = 0x05;
    sensor->i2c_commands[1] = 0x06;
    sensor->i2c_commands[2] = 0x07;

    sensor->current_channel = 0;
    sensor->enabled = false;
}

void ColorSensor_EnableStatus(colorsensor* sensor, bool enable)
{
	sensor->enabled = enable;
}

void ColorSensor_RGBStartColorReceive(colorsensor* sensor)
{
    if (HAL_I2C_IsDeviceReady(sensor->i2c, sensor->slave_address, 1, 1) == HAL_OK && sensor->enabled) {
        HAL_I2C_Master_Transmit(sensor->i2c, sensor->slave_address, &sensor->i2c_commands[sensor->current_channel], 1, HAL_MAX_DELAY);
        HAL_I2C_Master_Receive_DMA(sensor->i2c, sensor->slave_address, (uint8_t *)&sensor->rgb_data[sensor->current_channel], 2);
    }
}

void ColorSensor_ReceiveInterrupt(colorsensor* sensor)
{
	if (sensor->current_channel == 2)
	{
		ColorSensor_Handle(sensor);
	}

	sensor->current_channel++;
	sensor->current_channel %= 3;

	ColorSensor_RGBStartColorReceive(sensor);
}

color ColorSensor_CalculateColor(colorsensor* sensor)
{
	uint16_t max_val = sensor->rgb_data[0];
	color detected_color = RED;

	if (sensor->rgb_data[1] > max_val) detected_color = GREEN;
	if (sensor->rgb_data[2] > max_val) detected_color = BLUE;

	return detected_color;
}

void ColorSensor_Handle(colorsensor* sensor)
{
	color detected_color = ColorSensor_CalculateColor(sensor);
}

