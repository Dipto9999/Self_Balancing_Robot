#include "distancesensor.h"

#define TIM_PERIOD 65535
#define MAX_DISTANCE 813
#define MIN_DISTANCE 20

extern speaker Speaker;

void DistanceSensor_Init(distancesensor* sensor, TIM_HandleTypeDef* timer, feature_id ID, GPIO_TypeDef* icGPIOPort, uint16_t icGPIOPin, GPIO_TypeDef* statusGPIOPort, uint16_t statusGPIOPin)
{
	sensor->timer = timer;

	sensor->timeDifference = 0;
	sensor->countAtMinDistance = 0;

	sensor->ID = ID;

	sensor->icGPIOPort = icGPIOPort;
	sensor->icGPIOPin = icGPIOPin;

	sensor->statusGPIOPort = statusGPIOPort;
	sensor->statusGPIOPin = statusGPIOPin;

	HAL_GPIO_WritePin(sensor->statusGPIOPort, sensor->statusGPIOPin, GPIO_PIN_SET);


}

void DistanceSensor_Start(distancesensor* sensor)
{
	HAL_TIM_PWM_Start(sensor->timer, TIM_CHANNEL_2);
	HAL_TIM_IC_Start_IT(sensor->timer, TIM_CHANNEL_1);
}

void DistanceSensor_Stop(distancesensor* sensor)
{
	HAL_TIM_PWM_Stop(sensor->timer, TIM_CHANNEL_2);
	HAL_TIM_IC_Stop_IT(sensor->timer, TIM_CHANNEL_1);
}

void DistanceSensor_InputCaptureInterrupt(distancesensor* sensor)
{

		if (HAL_GPIO_ReadPin(sensor->icGPIOPort, sensor->icGPIOPin)) {
			sensor->IC_Value1 = HAL_TIM_ReadCapturedValue(sensor->timer, TIM_CHANNEL_1); // First rising edge
			HAL_TIM_PWM_Stop(sensor->timer, TIM_CHANNEL_2);
		}
		else
		{
			sensor->IC_Value2 = HAL_TIM_ReadCapturedValue(sensor->timer, TIM_CHANNEL_1); // Second rising edge
			if (sensor->IC_Value2 > sensor->IC_Value1) {
				sensor->timeDifference = sensor->IC_Value2 - sensor->IC_Value1;
			}
			else {
				sensor->timeDifference = (TIM_PERIOD + 1 - sensor->IC_Value1) + sensor->IC_Value2; // Handle overflow
			}

			HAL_TIM_PWM_Start(sensor->timer, TIM_CHANNEL_2);
			__HAL_TIM_SetCounter(sensor->timer, 65535);

			DistanceSensor_Handle(sensor);

		}


}

float DistanceSensor_GetDistance(distancesensor* sensor)
{
	// Number of microseconds / 58 = distance in cm
	return (float) sensor->timeDifference / 58;
}

void DistanceSensor_Handle(distancesensor* sensor)
{
	float distance = DistanceSensor_GetDistance(sensor);

	if (distance > MIN_DISTANCE)
	{
		sensor->countAtMinDistance = 0;
		HAL_GPIO_WritePin(sensor->statusGPIOPort, sensor->statusGPIOPin, GPIO_PIN_SET);
		//HAL_GPIO_WritePin(DISTANCE_SENSOR_FRONT_STATUS_GPIO_Port, DISTANCE_SENSOR_FRONT_STATUS_Pin, GPIO_PIN_SET);

		if (Speaker.hasFault)
			Speaker_Stop(&Speaker, sensor->ID);
	}
	else if (sensor->countAtMinDistance > 2)
	{
		HAL_GPIO_WritePin(sensor->statusGPIOPort, sensor->statusGPIOPin, GPIO_PIN_RESET);
		//HAL_GPIO_WritePin(DISTANCE_SENSOR_FRONT_STATUS_GPIO_Port, DISTANCE_SENSOR_FRONT_STATUS_Pin, GPIO_PIN_RESET);

		if (!Speaker.hasFault)
			Speaker_Start(&Speaker, sensor->ID);
	}
	else
	{
		sensor->countAtMinDistance++;
	}

}

