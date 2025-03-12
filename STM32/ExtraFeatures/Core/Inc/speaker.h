#ifndef INC_SPEAKER_H_
#define INC_SPEAKER_H_

#include "main.h"
#include <stdbool.h>

typedef struct
{
	TIM_HandleTypeDef* timer;

	volatile uint16_t beepLengthOn;
	volatile uint16_t beepLengthPeriod;
	volatile uint8_t wantedNumBeeps;
	volatile uint8_t currentNumBeeps;
	volatile uint16_t timerCounter;


	volatile bool featureFault[4];
	volatile bool hasFault;

} speaker;

void Speaker_Init(speaker* speaker, TIM_HandleTypeDef* timer);
void Speaker_Start(speaker* speaker, uint8_t ID);
void Speaker_Stop(speaker* speaker, uint8_t ID);
bool Speaker_Beep(speaker* speaker, uint16_t length_on_ms, uint16_t length_off_ms, uint8_t numBeeps);
void Speaker_BeepInterrupt(speaker* speaker);
void Speaker_SetAutoReload(speaker* speaker, uint16_t value);


#endif /* INC_SPEAKER_H_ */
