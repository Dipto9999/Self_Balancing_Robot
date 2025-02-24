#ifndef INC_SPEAKER_H_
#define INC_SPEAKER_H_

#include "main.h"

void Speaker_Start(TIM_HandleTypeDef* timer);
void Speaker_Stop(TIM_HandleTypeDef* timer);
void Speaker_SetAutoReload(TIM_HandleTypeDef* timer, uint16_t value);


#endif /* INC_SPEAKER_H_ */
