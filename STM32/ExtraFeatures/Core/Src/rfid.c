#include "rfid.h"

#define SERIALNUM_1_IDLE 32

#define SERIALNUM_0_CORRECT_CARD 170
#define SERIALNUM_1_CORRECT_CARD 205
#define SERIALNUM_2_CORRECT_CARD 47
#define SERIALNUM_3_CORRECT_CARD 3
#define SERIALNUM_4_CORRECT_CARD 75

extern speaker Speaker;
extern UART_HandleTypeDef huart1;
extern char Data;

void RFID_Init(rfid* sensor) {
    MFRC522_Init();
    memset(sensor->prevSerialNum, 0, 5);
    sensor->status = CARD_IDLE;

    sensor->initialSuccessfulCardTap = true;
}

rfid_card_status RFID_ValidateCard(rfid* sensor)
{
	uint8_t serialNum[5];
	MFRC522_Request(PICC_REQIDL, serialNum);
	MFRC522_Anticoll(serialNum);

	sensor->status = CARD_IDLE;

	if ((serialNum[0] == 170 && serialNum[1] == 205 && serialNum[2] == 47 && serialNum[3] == 3 && serialNum[4] == 75) ||
			(sensor->prevSerialNum[0] == 170 && sensor->prevSerialNum[1] == 205 && sensor->prevSerialNum[2] == 47 && sensor->prevSerialNum[3] == 3 && sensor->prevSerialNum[4] == 75))
	{
		sensor->status = CARD_SUCCESS;

	}
	else if (!(serialNum[0] | serialNum[1] | serialNum[2] | serialNum[3] | serialNum[4]))
	{
		if (!(sensor->prevSerialNum[0] | sensor->prevSerialNum[1] | sensor->prevSerialNum[2] | sensor->prevSerialNum[3] | sensor->prevSerialNum[4]))
			sensor->status = CARD_IDLE;
		else
		{
			sensor->status = CARD_FAIL;
		}

	}
	else
	{
		sensor->status = CARD_FAIL;
	}


	for (uint8_t i = 0; i < 5; i++)
	{
		sensor->prevSerialNum[i] = serialNum[i];
	}

	return sensor->status;
}

void RFID_SecurityLogic(rfid* sensor)
{
	rfid_card_status cardStatus = RFID_ValidateCard(sensor);

	switch (cardStatus) {
	    case CARD_SUCCESS:
	    	if (sensor->initialSuccessfulCardTap)
	    	{

	    		sensor->initialSuccessfulCardTap = false;
	    	}
	        break;

	    case CARD_FAIL:
	        sensor->initialSuccessfulCardTap = true;
	        break;

	    case CARD_IDLE:
	    	sensor->initialSuccessfulCardTap = true;
	        break;

	    // Optional: Default case if no case matches
	    default:
	        // Code to execute if none of the above cases match
	        break;
	}

}


