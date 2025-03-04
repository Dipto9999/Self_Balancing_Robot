#ifndef INC_SPEAKER_H_
#define INC_SPEAKER_H_

#include "main.h"
#include <stdbool.h>

typedef struct
{
	TIM_HandleTypeDef* timer;
	volatile bool isActive;

	volatile bool featureFault[4];
} speaker;

void Speaker_Init(speaker* speaker, TIM_HandleTypeDef* timer);
void Speaker_Start(speaker* speaker, uint8_t ID);
void Speaker_Stop(speaker* speaker, uint8_t ID);
void Speaker_SetAutoReload(speaker* speaker, uint16_t value);


#endif /* INC_SPEAKER_H_ */
