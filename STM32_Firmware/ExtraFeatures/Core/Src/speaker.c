#include "speaker.h"


#define CLK_SPEED 32000000
#define DEFAULT_AUTORELOAD 488

void Speaker_Init(speaker* speaker, rfid* rfid_struct, TIM_HandleTypeDef* timer)
{
	speaker->rfid_sensor = rfid_struct;
	speaker->timer = timer;

	speaker->hasFault = false;
	speaker->beepLengthOn = 0;
	speaker->beepLengthPeriod = 0;
	speaker->wantedNumBeeps = 0;
	speaker->currentNumBeeps = 0;
	speaker->timerCounter = 0;

	for (uint8_t i = 0; i < sizeof(speaker->featureFault) / sizeof(speaker->featureFault[0]); i++)
	{
		speaker->featureFault[i] = false;
	}


}

void Speaker_Start(speaker* speaker, uint8_t ID)
{

	speaker->featureFault[ID] = true;
	if ((speaker->featureFault[0] || speaker->featureFault[1] || speaker->featureFault[2]) && speaker->rfid_sensor->botEnabled)
	{
		speaker->hasFault = true;
		Speaker_SetAutoReload(speaker, DEFAULT_AUTORELOAD);
		HAL_TIM_PWM_Start(speaker->timer, TIM_CHANNEL_1);
	}

}

void Speaker_Stop(speaker* speaker, uint8_t ID)
{
	speaker->featureFault[ID] = false;
	if (!(speaker->featureFault[0] || speaker->featureFault[1] || speaker->featureFault[2]))
	{
		speaker->hasFault = false;
		HAL_TIM_PWM_Stop(speaker->timer, TIM_CHANNEL_1);
	}
}



bool Speaker_Beep(speaker* speaker, uint16_t length_on_ms, uint16_t length_off_ms, uint8_t numBeeps)
{

	if (speaker->hasFault)
		return false;


	speaker->timerCounter = 0;
	speaker->currentNumBeeps = 0;



	speaker->beepLengthOn = length_on_ms * 1000 / __HAL_TIM_GET_AUTORELOAD(speaker->timer);
	speaker->beepLengthPeriod =speaker->beepLengthOn + length_off_ms * 1000 / __HAL_TIM_GET_AUTORELOAD(speaker->timer);
	speaker->wantedNumBeeps = numBeeps;

	//speaker->beepLength = length_ms * CLK_SPEED / (speaker->timer->Instance->PSC);

	HAL_TIM_Base_Start_IT(speaker->timer);
	HAL_TIM_PWM_Start(speaker->timer, TIM_CHANNEL_1);
	return true;
}
/*
bool Speaker_IsTimerRunning(speaker* speaker)
{
	return ((HAL_TIM_Base_GetState(speaker->timer) == HAL_TIM_STATE_BUSY) ? true : false);
}
*/

void Speaker_BeepInterrupt(speaker* speaker)
{
    if (speaker->currentNumBeeps < speaker->wantedNumBeeps)
    {

        if (speaker->timerCounter == speaker->beepLengthOn)
        {
            HAL_TIM_PWM_Stop(speaker->timer, TIM_CHANNEL_1);
            __HAL_TIM_ENABLE(speaker->timer);
        	//__NOP();
        }
        else if (speaker->timerCounter >= speaker->beepLengthPeriod)
        {
        	speaker->currentNumBeeps++;
			speaker->timerCounter = 0;

            if (speaker->currentNumBeeps < speaker->wantedNumBeeps)
            {
                HAL_TIM_PWM_Start(speaker->timer, TIM_CHANNEL_1);

            	//__NOP();
            }

        }
        speaker->timerCounter++;
    }
    else
    {
        HAL_TIM_PWM_Stop(speaker->timer, TIM_CHANNEL_1);
        HAL_TIM_Base_Stop_IT(speaker->timer);
    }
}

void Speaker_SetAutoReload(speaker* speaker, uint16_t value)
{
	__HAL_TIM_SET_AUTORELOAD(speaker->timer, value);
	__HAL_TIM_SET_COMPARE(speaker->timer, TIM_CHANNEL_1, value / 2);
}

