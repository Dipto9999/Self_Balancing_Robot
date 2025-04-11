#include "speaker.h"

void Speaker_Init(speaker* speaker, TIM_HandleTypeDef* timer)
{
	speaker->timer = timer;
	speaker->isActive = true;

	for (uint8_t i = 0; i < sizeof(speaker->featureFault) / sizeof(speaker->featureFault[0]); i++)
	{
		speaker->featureFault[i] = false;
	}


}

void Speaker_Start(speaker* speaker, uint8_t ID)
{
	speaker->featureFault[ID] = true;
	if (speaker->featureFault[0] && speaker->featureFault[1] && speaker->featureFault[2] && speaker->featureFault[3])
	{
		speaker->isActive = true;
		HAL_TIM_PWM_Start(speaker->timer, TIM_CHANNEL_1);
	}

}

void Speaker_Stop(speaker* speaker, uint8_t ID)
{
	speaker->featureFault[ID] = false;
	if (!(speaker->featureFault[0] || speaker->featureFault[1] || speaker->featureFault[2] || speaker->featureFault[3]))
	{
		speaker->isActive = false;
		HAL_TIM_PWM_Stop(speaker->timer, TIM_CHANNEL_1);
	}
}

void Speaker_SetAutoReload(speaker* speaker, uint16_t value)
{
	__HAL_TIM_SET_AUTORELOAD(speaker->timer, value);
	__HAL_TIM_SET_COMPARE(speaker->timer, TIM_CHANNEL_1, value / 2);
}

