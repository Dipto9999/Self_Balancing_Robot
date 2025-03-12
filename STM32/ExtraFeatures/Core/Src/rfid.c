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

    sensor->botEnabled = false;
    sensor->initialSuccessfulCardTap = true;
    sensor->initialFailedCardTap = true;
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
	else if (!(serialNum[0] | (!(serialNum[1] == 32 || serialNum[1] == 0)) | serialNum[2] | serialNum[3] | serialNum[4]))
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

	    		Speaker_SetAutoReload(&Speaker, 488);
	    		Speaker_Beep(&Speaker, 150, 0, 1);
	    		sensor->initialSuccessfulCardTap = false;
	    		sensor->initialFailedCardTap = true;
	    		sensor->botEnabled = !sensor->botEnabled;
	    	}
	        break;

	    case CARD_FAIL:
	    	if (sensor->initialFailedCardTap)
	    	{
	    		HAL_Delay(200);
				if (RFID_ValidateCard(sensor) != CARD_FAIL)
					break;
				Speaker_SetAutoReload(&Speaker, 488 * 4);
				Speaker_Beep(&Speaker, 150, 50, 4);
				sensor->initialSuccessfulCardTap = true;
				sensor->initialFailedCardTap = false;
	    	}
	    	//HAL_Delay(HAL_MAX_DELAY);

	        break;

	    case CARD_IDLE:
	    	sensor->initialSuccessfulCardTap = true;
	    	sensor->initialFailedCardTap = true;
	        break;

	    // Optional: Default case if no case matches
	    default:
	        // Code to execute if none of the above cases match
	        break;
	}

}


