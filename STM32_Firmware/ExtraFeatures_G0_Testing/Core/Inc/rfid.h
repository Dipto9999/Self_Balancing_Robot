#ifndef INC_RFID_H_
#define INC_RFID_H_

#include "main.h"

typedef enum {
    // Page 0: Command and status
    CommandReg               = 0x01 << 1,    // starts and stops command execution
    ComIEnReg                = 0x02 << 1,    // enable and disable interrupt request control bits
    DivIEnReg                = 0x03 << 1,    // enable and disable interrupt request control bits
    ComIrqReg                = 0x04 << 1,    // interrupt request bits
    DivIrqReg                = 0x05 << 1,    // interrupt request bits
    ErrorReg                = 0x06 << 1,    // error bits showing the error status of the last command executed
    Status1Reg              = 0x07 << 1,    // communication status bits
    Status2Reg              = 0x08 << 1,    // receiver and transmitter status bits
    FIFODataReg             = 0x09 << 1,    // input and output of 64 byte FIFO buffer
    FIFOLevelReg            = 0x0A << 1,    // number of bytes stored in the FIFO buffer
    WaterLevelReg           = 0x0B << 1,    // level for FIFO underflow and overflow warning
    ControlReg              = 0x0C << 1,    // miscellaneous control registers
    BitFramingReg           = 0x0D << 1,    // adjustments for bit-oriented frames
    CollReg                 = 0x0E << 1,    // bit position of the first bit-collision detected on the RF interface

    // Page 1: Command
    ModeReg                 = 0x11 << 1,    // defines general modes for transmitting and receiving
    TxModeReg               = 0x12 << 1,    // defines transmission data rate and framing
    RxModeReg               = 0x13 << 1,    // defines reception data rate and framing
    TxControlReg            = 0x14 << 1,    // controls the logical behavior of the antenna driver pins TX1 and TX2
    TxASKReg                = 0x15 << 1,    // controls the setting of the transmission modulation
    TxSelReg                = 0x16 << 1,    // selects the internal sources for the antenna driver
    RxSelReg                = 0x17 << 1,    // selects internal receiver settings
    RxThresholdReg          = 0x18 << 1,    // selects thresholds for the bit decoder
    DemodReg                = 0x19 << 1,    // defines demodulator settings
    MfTxReg                 = 0x1C << 1,    // controls some MIFARE communication transmit parameters
    MfRxReg                 = 0x1D << 1,    // controls some MIFARE communication receive parameters
    SerialSpeedReg          = 0x1F << 1,    // selects the speed of the serial UART interface

    // Page 2: Configuration
    CRCResultRegH           = 0x21 << 1,    // shows the MSB and LSB values of the CRC calculation
    CRCResultRegL           = 0x22 << 1,
    ModWidthReg             = 0x24 << 1,    // controls the ModWidth setting?
    RFCfgReg                = 0x26 << 1,    // configures the receiver gain
    GsNReg                  = 0x27 << 1,    // selects the conductance of the antenna driver pins TX1 and TX2 for modulation
    CWGsPReg                = 0x28 << 1,    // defines the conductance of the p-driver output during periods of no modulation
    ModGsPReg               = 0x29 << 1,    // defines the conductance of the p-driver output during periods of modulation
    TModeReg                = 0x2A << 1,    // defines settings for the internal timer
    TPrescalerReg           = 0x2B << 1,    // the lower 8 bits of the TPrescaler value. The 4 high bits are in TModeReg.
    TReloadRegH             = 0x2C << 1,    // defines the 16-bit timer reload value
    TReloadRegL             = 0x2D << 1,
    TCounterValueRegH       = 0x2E << 1,    // shows the 16-bit timer value
    TCounterValueRegL       = 0x2F << 1,

    // Page 3: Test Registers
    TestSel1Reg             = 0x31 << 1,    // general test signal configuration
    TestSel2Reg             = 0x32 << 1,    // general test signal configuration
    TestPinEnReg            = 0x33 << 1,    // enables pin output driver on pins D1 to D7
    TestPinValueReg         = 0x34 << 1,    // defines the values for D1 to D7 when it is used as an I/O bus
    TestBusReg              = 0x35 << 1,    // shows the status of the internal test bus
    AutoTestReg             = 0x36 << 1,    // controls the digital self-test
    VersionReg              = 0x37 << 1,    // shows the software version
    AnalogTestReg           = 0x38 << 1,    // controls the pins AUX1 and AUX2
    TestDAC1Reg             = 0x39 << 1,    // defines the test value for TestDAC1
    TestDAC2Reg             = 0x3A << 1,    // defines the test value for TestDAC2
    TestADCReg              = 0x3B << 1     // shows the value of ADC I and Q channels
} rfid_register;


typedef enum {
		CommandReg_Idle					= 0x00,		// no action, cancels current command execution
		CommandReg_Mem					= 0x01,		// stores 25 bytes into the internal buffer
		CommandReg_GenerateRandomID		= 0x02,		// generates a 10-byte random ID number
		CommandReg_CalcCRC				= 0x03,		// activates the CRC coprocessor or performs a self-test
		CommandReg_Transmit				= 0x04,		// transmits data from the FIFO buffer
		CommandReg_NoCmdChange			= 0x07,		// no command change, can be used to modify the CommandReg register bits without affecting the command, for example, the PowerDown bit
		CommandReg_Receive				= 0x08,		// activates the receiver circuits

		CommandReg_Anticoll          	= 0x12,
		CommandReg_Transceive 			= 0x0C,		// transmits data from FIFO buffer to antenna and automatically activates the receiver after transmission
		CommandReg_MFAuthent 			= 0x0E,		// performs the MIFARE standard authentication as a reader
		CommandReg_SoftReset			= 0x0F		// resets the MFRC522
} rfid_commandreg_commands;

// Define RFID Operation Status as an Enum
typedef enum {
    RFID_OK = 0x00,            // Successful operation
    RFID_ERR = 0x01,           // General error in operation
    RFID_ERR_TIMEOUT = 0x02,   // Timeout error
    RFID_ERR_NO_TAG = 0x03,    // No tag detected
    RFID_ERR_COLLISION = 0x04  // Collision detected
} rfid_status;


typedef struct
{
	SPI_HandleTypeDef* spi;

} rfid;


void RFID_Init(rfid* sensor, SPI_HandleTypeDef* spi_handle);
void RFID_WriteRegister(rfid* sensor, rfid_register reg, uint8_t value);
void RFID_ReadRegister(rfid* sensor, rfid_register reg, uint8_t count, uint8_t *values, uint8_t rxAlign);
void RFID_AntennaOn(rfid* sensor);
void RFID_SoftReset(rfid* sensor);
uint8_t RFID_Request(rfid* sensor, uint8_t reqMode, uint8_t* tagType);
uint8_t RFID_Anticoll(rfid* sensor, uint8_t* serial);
uint8_t RFID_Communicate(rfid* sensor, uint8_t command, uint8_t* sendData, uint8_t sendLen, uint8_t* backData, uint8_t* backLen);


#endif /* INC_RFID_H_ */
