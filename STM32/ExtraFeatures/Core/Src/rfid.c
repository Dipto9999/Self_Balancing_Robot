#include "rfid.h"

void RFID_Init(rfid* sensor, SPI_HandleTypeDef* spi_handle) {
    sensor->spi = spi_handle;

    // Reset baud rates
	RFID_WriteRegister(sensor, TxModeReg, 0x00);
	RFID_WriteRegister(sensor, RxModeReg, 0x00);
	// Reset ModWidthReg
	RFID_WriteRegister(sensor, ModWidthReg, 0x26);

	// When communicating with a PICC we need a timeout if something goes wrong.
	// f_timer = 13.56 MHz / (2*TPreScaler+1) where TPreScaler = [TPrescaler_Hi:TPrescaler_Lo].
	// TPrescaler_Hi are the four low bits in TModeReg. TPrescaler_Lo is TPrescalerReg.
	RFID_WriteRegister(sensor, TModeReg, 0x80);			// TAuto=1; timer starts automatically at the end of the transmission in all communication modes at all speeds
	RFID_WriteRegister(sensor, TPrescalerReg, 0xA9);		// TPreScaler = TModeReg[3..0]:TPrescalerReg, ie 0x0A9 = 169 => f_timer=40kHz, ie a timer period of 25μs.
	RFID_WriteRegister(sensor, TReloadRegH, 0x03);		// Reload timer with 0x3E8 = 1000, ie 25ms before timeout.
	RFID_WriteRegister(sensor, TReloadRegL, 0xE8);

	RFID_WriteRegister(sensor, TxASKReg, 0x40);		// Default 0x00. Force a 100 % ASK modulation independent of the ModGsPReg register setting
	RFID_WriteRegister(sensor, ModeReg, 0x3D);		// Default 0x3F. Set the preset value for the CRC coprocessor for the CalcCRC command to 0x6363 (ISO 14443-3 part 6.2.4)
	RFID_AntennaOn(sensor);						// Enable the antenna driver pins TX1 and TX2 (they were disabled by the reset)
}

void RFID_WriteRegister(rfid* sensor, rfid_register reg, uint8_t value)
{

	HAL_SPI_Transmit(sensor->spi, &value, 1, HAL_MAX_DELAY);
}

void RFID_ReadRegister(rfid* sensor, rfid_register reg, uint8_t count, uint8_t *values, uint8_t rxAlign) {
    uint8_t address = (reg & 0x7E) | 0x80; // Ensure proper read command (MSB = 1)

    HAL_SPI_Transmit(sensor->spi, &address, 1, HAL_MAX_DELAY); // Send register address

    for (uint8_t i = 0; i < count; i++) {
        uint8_t receivedByte;
        HAL_SPI_Receive(sensor->spi, &receivedByte, 1, HAL_MAX_DELAY);

        if (i == 0 && rxAlign) {
            uint8_t mask = (0xFF << rxAlign) & 0xFF;
            values[i] = (values[i] & ~mask) | (receivedByte & mask);
        } else {
            values[i] = receivedByte;
        }
    }

}


void RFID_AntennaOn(rfid* sensor) {
    uint8_t value;

    // Read current TxControlReg value
    RFID_ReadRegister(sensor, TxControlReg, 1, &value, 0);

    // Check if the two lowest bits (0x03) are set
    if ((value & 0x03) != 0x03)
    {
        value |= 0x03; // Set the bits to enable the antenna
        RFID_WriteRegister(sensor, TxControlReg, value);
    }
}



void RFID_SoftReset(rfid* sensor) {
    RFID_WriteRegister(sensor, CommandReg, SoftReset);
    HAL_Delay(50);
}
/*
uint8_t RFID_Request(rfid* sensor, uint8_t reqMode, uint8_t* tagType) {
    uint8_t status;
    uint8_t backData[2];
    uint8_t backLen;
    uint8_t validBits = 7;

    RFID_WriteRegister(sensor, BitFramingReg, validBits);
    status = RFID_Communicate(sensor, Transceive, &reqMode, 1, backData, &backLen);

    if (status == RFID_OK && backLen == 16) {
        tagType[0] = backData[0];
        tagType[1] = backData[1];
    }
    return status;
}

uint8_t RFID_Anticoll(rfid* sensor, uint8_t* serial) {
    uint8_t status;
    uint8_t backData[5];
    uint8_t backLen;
    uint8_t serialNumber[] = { MFRC522_PICC_ANTICOLL, 0x20 };

    RFID_WriteRegister(sensor, BitFramingReg, 0x00);
    status = RFID_Communicate(sensor, Transceive, serialNumber, 2, backData, &backLen);

    if (status == RFID_OK && backLen == 40) {
        for (uint8_t i = 0; i < 5; i++) {
            serial[i] = backData[i];
        }
    }
    return status;
}

uint8_t RFID_Communicate(rfid* sensor, uint8_t command, uint8_t* sendData, uint8_t sendLen, uint8_t* backData, uint8_t* backLen) {
    uint8_t status = RFID_ERR;
    uint8_t irqEn = 0x77;
    uint8_t waitIRq = 0x30;

    RFID_WriteRegister(sensor, ComIEnReg, irqEn | 0x80);
    RFID_WriteRegister(sensor, CommandReg, MFRC522_CMD_Idle);
    RFID_WriteRegister(sensor, FIFOLevelReg, 0x80);
    for (uint8_t i = 0; i < sendLen; i++) {
        RFID_WriteRegister(sensor, FIFODataReg, sendData[i]);
    }
    RFID_WriteRegister(sensor, CommandReg, command);

    if (command == MFRC522_CMD_Transceive) {
        RFID_WriteRegister(sensor, BitFramingReg, 0x80);
    }

    uint16_t i = 2000;
    while (i--) {
        uint8_t n = 0;
        RFID_ReadRegister(sensor, ComIrqReg, 1, &n, 0);
        if (n & waitIRq) {
            break;
        }
    }

    uint8_t errorReg = 0;
    RFID_ReadRegister(sensor, ErrorReg, 1, &errorReg, 0);
    if (!(errorReg & 0x1B)) {
        status = RFID_OK;
        if (backData && backLen) {
            RFID_ReadRegister(sensor, FIFOLevelReg, 1, backLen, 0);
            for (uint8_t i = 0; i < *backLen; i++) {
                RFID_ReadRegister(sensor, FIFODataReg, 1, &backData[i], 0);
            }
        }
    }
    return status;
}
*/
