#ifndef INC_DISTANCESENSOR_H_
#define INC_DISTANCESENSOR_H_

#include "main.h"
#include <stdbool.h>

typedef struct
{
	TIM_HandleTypeDef* timer;

	uint16_t IC_Value1;
	uint16_t IC_Value2;
	uint32_t timeDifference;

	uint8_t icTimerChannel;
	uint8_t pwmTimerChannel;
	uint8_t captureCompareFlag;
} distancesensor;


void DistanceSensor_Init(distancesensor* sensor, TIM_HandleTypeDef* timer, uint8_t icTimerChannel, uint8_t ocTimerChannel, uint8_t captureCompareFlag);
void DistanceSensor_Start(distancesensor* sensor);
void DistanceSensor_Stop(distancesensor* sensor);
void DistanceSensor_InputCaptureInterrupt(distancesensor* sensor);
float DistanceSensor_GetDistance(distancesensor* sensor);



#endif
