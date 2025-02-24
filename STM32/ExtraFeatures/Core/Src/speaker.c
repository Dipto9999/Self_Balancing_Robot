#include "speaker.h"

void Speaker_Start(TIM_HandleTypeDef* timer)
{
	HAL_TIM_PWM_Start(timer, TIM_CHANNEL_1);
}

void Speaker_Stop(TIM_HandleTypeDef* timer)
{
	HAL_TIM_PWM_Stop(timer, TIM_CHANNEL_1);
}

void Speaker_SetAutoReload(TIM_HandleTypeDef* timer, uint16_t value)
{
	__HAL_TIM_SET_AUTORELOAD(timer, value);
	__HAL_TIM_SET_COMPARE(timer, TIM_CHANNEL_1, value / 2);
}

