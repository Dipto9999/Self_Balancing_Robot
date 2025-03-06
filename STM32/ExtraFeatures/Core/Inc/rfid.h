#ifndef INC_RFID_H_
#define INC_RFID_H_

#include "main.h"

#include "rc522.h"
#include "speaker.h"

#include <string.h>
#include <stdio.h>

typedef enum
{
	CARD_SUCCESS,
	CARD_FAIL,
	CARD_IDLE
} rfid_card_status;

typedef struct
{
	uint8_t prevSerialNum[5];
	rfid_card_status status;

	bool initialSuccessfulCardTap;
} rfid;

void RFID_Init(rfid* sensor);
rfid_card_status RFID_ValidateCard(rfid* sensor);


#endif /* INC_RFID_H_ */
