#ifndef INC_SPEAKER_H_
#define INC_SPEAKER_H_

#include "main.h"
#include <stdbool.h>

typedef struct
{
	TIM_HandleTypeDef* timer;
	volatile bool isActive;
} speaker;

void Speaker_Init(speaker* speaker, TIM_HandleTypeDef* timer);
void Speaker_Start(speaker* speaker);
void Speaker_Stop(speaker* speaker);
void Speaker_SetAutoReload(speaker* speaker, uint16_t value);


#endif /* INC_SPEAKER_H_ */
