#ifndef INC_DISTANCESENSOR_H_
#define INC_DISTANCESENSOR_H_

#include "main.h"
#include "speaker.h"
#include <stdbool.h>

typedef struct
{
	TIM_HandleTypeDef* timer;

	uint16_t IC_Value1;
	uint16_t IC_Value2;
	uint32_t timeDifference;

	uint8_t countAtMaxDistance;

	feature_id ID;

	GPIO_TypeDef* icGPIOPort;
	uint8_t icGPIOPin;

	GPIO_TypeDef* statusGPIOPort;
	uint8_t statusGPIOPin;
} distancesensor;


void DistanceSensor_Init(distancesensor* sensor, TIM_HandleTypeDef* timer, feature_id ID, GPIO_TypeDef* icGPIOPort, uint8_t icGPIOPin, GPIO_TypeDef* statusGPIOPort, uint8_t statusGPIOPin);
void DistanceSensor_Start(distancesensor* sensor);
void DistanceSensor_Stop(distancesensor* sensor);
void DistanceSensor_InputCaptureInterrupt(distancesensor* sensor);
float DistanceSensor_GetDistance(distancesensor* sensor);
void DistanceSensor_Handle(distancesensor* sensor);



#endif
