#include "distancesensor.h"

#define TIM_PERIOD 65535

void DistanceSensor_Init(distancesensor* sensor, TIM_HandleTypeDef* timer, uint8_t icTimerChannel, uint8_t ocTimerChannel, uint8_t captureCompareFlag)
{
	sensor->timer = timer;
	sensor->icTimerChannel = icTimerChannel;
	sensor->ocTimerChannel = ocTimerChannel;
	sensor->captureCompareFlag = captureCompareFlag;


}

void DistanceSensor_Start(distancesensor* sensor)
{
	sensor->IC_Value1 = 0;
	sensor->risingEdge = true;

	HAL_TIM_IC_Start_IT(sensor->timer, sensor->icTimerChannel);
	HAL_TIM_OnePulse_Start(sensor->timer, sensor->ocTimerChannel);
}

void DistanceSensor_Stop(distancesensor* sensor)
{
	HAL_TIM_IC_Stop_IT(sensor->timer, sensor->icTimerChannel);
}

void DistanceSensor_InputCaptureInterrupt(distancesensor* sensor)
{

	if (__HAL_TIM_GET_FLAG(sensor->timer, sensor->captureCompareFlag))
	{



		if (sensor->risingEdge) {
			sensor->IC_Value1 = HAL_TIM_ReadCapturedValue(sensor->timer, sensor->icTimerChannel); // First rising edge
			sensor->risingEdge = false;
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
			sensor->risingEdge = true;
		}
	}

	HAL_TIM_OnePulse_Start(sensor->timer, sensor->ocTimerChannel);
}

float DistanceSensor_GetDistance(distancesensor* sensor)
{
	// Number of microseconds / 58 = distance in cm
	return (float) sensor->timeDifference / 58;
}

