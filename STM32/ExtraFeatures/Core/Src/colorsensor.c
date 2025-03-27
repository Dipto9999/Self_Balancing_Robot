#include "colorsensor.h"

extern speaker Speaker;

void ColorSensor_Init(colorsensor* sensor, I2C_HandleTypeDef* i2c_handle) {
    sensor->i2c = i2c_handle;
    sensor->slave_address = TCS3472_SLAVE_ADDRESS;

    memset(sensor->rgb_data, 0, sizeof(sensor->rgb_data));  // Clear RGB data
    sensor->enabled = false;

    // Define initialization sequence
    uint8_t init_data[][2] = {
        { TCS3472_WTIME_REG  | 0x80, 0xFF },  // Wait time
        { TCS3472_ATIME_REG  | 0x80, 0xFF },  // Integration time
        { TCS3472_ENABLE_REG | 0x80, 0x0B },   // Power ON & enable RGBC
    };

    // Loop through initialization commands and send them
    for (size_t i = 0; i < sizeof(init_data) / sizeof(init_data[0]); i++) {
        if (HAL_I2C_Master_Transmit(sensor->i2c, sensor->slave_address, init_data[i], 2, HAL_MAX_DELAY) != HAL_OK)
        	while(1);
    }
}


void ColorSensor_EnableStatus(colorsensor* sensor, bool enable)
{
	sensor->enabled = enable;
}


uint16_t ColorSensor_Read16(colorsensor* sensor, uint8_t reg) {
    uint8_t buffer[2];
    uint8_t command = reg | 0xA0;  // Add auto-increment bit

    // Send register address
    if (HAL_I2C_Master_Transmit(sensor->i2c, sensor->slave_address, &command, 1, HAL_MAX_DELAY) != HAL_OK) {
        return 0; // Handle error
    }

    // Read 2 bytes from the sensor
    if (HAL_I2C_Master_Receive(sensor->i2c, sensor->slave_address, buffer, 2, HAL_MAX_DELAY) != HAL_OK) {
        return 0; // Handle error
    }

    // Combine two bytes into a 16-bit value (LSB first)
    return (uint16_t)(buffer[0] | (buffer[1] << 8));
}

void ColorSensor_ReadAll(colorsensor* sensor) {
    sensor->rgb_data[0] = ColorSensor_Read16(sensor, TCS3472_CDATAL_REG); // TCS3472_CDATAL
    sensor->rgb_data[1]   = ColorSensor_Read16(sensor, TCS3472_RDATAL_REG); // TCS3472_RDATAL
    sensor->rgb_data[2] = ColorSensor_Read16(sensor, TCS3472_GDATAL_REG); // TCS3472_GDATAL
    sensor->rgb_data[3]  = ColorSensor_Read16(sensor, TCS3472_BDATAL_REG); // TCS3472_BDATAL
}


color ColorSensor_CalculateColor(colorsensor* sensor)
{
	uint16_t max_val = sensor->rgb_data[0];
	color detected_color = CLEAR;
	if (sensor->rgb_data[1] > max_val) detected_color = RED;
	if (sensor->rgb_data[2] > max_val) detected_color = GREEN;
	if (sensor->rgb_data[3] > max_val) detected_color = BLUE;

	return detected_color;
}

void ColorSensor_Handle(colorsensor* sensor)
{
	ColorSensor_ReadAll(sensor);
	color detected_color = ColorSensor_CalculateColor(sensor);
	if (detected_color == RED && !Speaker.hasFault)
	{
		Speaker_Start(&Speaker, COLOR_SENSOR_ID);
	}
	else
	{
		Speaker_Stop(&Speaker, COLOR_SENSOR_ID);
	}

}

