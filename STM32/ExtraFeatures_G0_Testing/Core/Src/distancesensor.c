#include "distancesensor.h"

#define TIM_PERIOD 65535

void DistanceSensor_Init(distancesensor* sensor, TIM_HandleTypeDef* timer, uint8_t icTimerChannel, uint8_t pwmTimerChannel, uint8_t captureCompareFlag)
{
	sensor->timer = timer;
	sensor->icTimerChannel = icTimerChannel;
	sensor->pwmTimerChannel = pwmTimerChannel;
	sensor->captureCompareFlag = captureCompareFlag;
	sensor->timeDifference = 0;
}

void DistanceSensor_Start(distancesensor* sensor)
{
	HAL_TIM_PWM_Start(sensor->timer, sensor->pwmTimerChannel);
	HAL_TIM_IC_Start_IT(sensor->timer, sensor->icTimerChannel);
}

void DistanceSensor_Stop(distancesensor* sensor)
{
	HAL_TIM_PWM_Stop(sensor->timer, sensor->pwmTimerChannel);
	HAL_TIM_IC_Stop_IT(sensor->timer, sensor->icTimerChannel);
}

void DistanceSensor_InputCaptureInterrupt(distancesensor* sensor)
{

	//if (__HAL_TIM_GET_FLAG(sensor->timer, sensor->captureCompareFlag))
	{



		if (HAL_GPIO_ReadPin(DISTANCE_SENSOR_FRONT_INPUT_CAPTURE_GPIO_Port, DISTANCE_SENSOR_FRONT_INPUT_CAPTURE_Pin)) {
			sensor->IC_Value1 = HAL_TIM_ReadCapturedValue(sensor->timer, sensor->icTimerChannel); // First rising edge
			HAL_TIM_PWM_Stop(sensor->timer, sensor->pwmTimerChannel);
		}
		else
		{
			sensor->IC_Value2 = HAL_TIM_ReadCapturedValue(sensor->timer, sensor->icTimerChannel); // Second rising edge
			if (sensor->IC_Value2 > sensor->IC_Value1) {
				sensor->timeDifference = sensor->IC_Value2 - sensor->IC_Value1;
			}
			else {
				sensor->timeDifference = (TIM_PERIOD + 1 - sensor->IC_Value1) + sensor->IC_Value2; // Handle overflow
			}

			HAL_TIM_PWM_Start(sensor->timer, sensor->pwmTimerChannel);


		}
	}



}


float DistanceSensor_GetDistance(distancesensor* sensor)
{
	// Number of microseconds / 58 = distance in cm
	return (float) sensor->timeDifference / 58;
}

