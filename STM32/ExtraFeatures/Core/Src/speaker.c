#include "speaker.h"

void Speaker_Init(speaker* speaker, TIM_HandleTypeDef* timer)
{
	speaker->timer = timer;
	speaker->isActive = true;
}

void Speaker_Start(speaker* speaker)
{
	speaker->isActive = true;
	HAL_TIM_PWM_Start(speaker->timer, TIM_CHANNEL_1);
}

void Speaker_Stop(speaker* speaker)
{
	HAL_TIM_PWM_Stop(speaker->timer, TIM_CHANNEL_1);
}

void Speaker_SetAutoReload(speaker* speaker, uint16_t value)
{
	__HAL_TIM_SET_AUTORELOAD(speaker->timer, value);
	__HAL_TIM_SET_COMPARE(speaker->timer, TIM_CHANNEL_1, value / 2);
}

