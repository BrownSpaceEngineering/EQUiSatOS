#include "isItOn.h"

/*
 * CHECK THE DATASHEET THERES TOO MANY VARIABLES IN THE MEMORY ADDRESS
 * THIS IS PROBABLY NOT CORRECT
 */
uint8_t CHIP_ADDR = 0b1010000;
uint8_t memLocation[2] = {0x00, 0x01};
	
void isItOnInit(void) {
	configure_i2c_standard(SERCOM4);
}

uint8_t readCount(void) {
	writeDataToAddress(memLocation, 2, CHIP_ADDR, true);

	uint8_t buffer[1] = {0x42};

	readFromAddress(buffer, 1, CHIP_ADDR, true);
	
	return buffer[0];
}

void incrementOnCount(void) {		
    
	uint8_t newCount = readCount() + 1;

    uint8_t outBuffer[3];

    outBuffer[0] = 0x0;
    outBuffer[1] = 0x0;
    outBuffer[2] = newCount;

    writeDataToAddress(outBuffer, 3, CHIP_ADDR, true);
}

void resetCount(void) {
	
	uint8_t newCount = 0x00;

	uint8_t outBuffer[3];

	outBuffer[0] = 0x00;
	outBuffer[1] = 0x00;
	outBuffer[2] = newCount;

	writeDataToAddress(outBuffer, 3, CHIP_ADDR, true);
}