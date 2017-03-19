/*
 * TCA9535_GPIO.c
 *
 * Created: 3/15/2017 9:17:59 PM
 *  Author: BSE
 */ 

#include "TCA9535_GPIO.h"

void TCA9535_init(void){
	uint16_t x = readTCA9535Levels();
	setIOMask(IOMask0,IOMask1);
}

//Set the mask for the configuration register
void setIOMask(uint8_t reg0, uint8_t reg1){
	uint8_t data0[2] = {CONFIG_REGISTER_0, reg0};
	uint8_t data1[2] = {CONFIG_REGISTER_1, reg1};
	
	writeDataToAddress(data0, 2, TCA_ADDR, TCA_SHOULD_STOP_WRITE);
	writeDataToAddress(data1, 2, TCA_ADDR, TCA_SHOULD_STOP_WRITE);
}

uint16_t readTCA9535Levels(void){
	uint8_t data[2] = {0x0, 0x0};
	
	readFromAddressAndMemoryLocation(&(data[0]), 1, TCA_ADDR, INPUTS_REGISTER_0, TCA_SHOULD_STOP_READ);
	readFromAddressAndMemoryLocation(&(data[1]), 1, TCA_ADDR, INPUTS_REGISTER_1, TCA_SHOULD_STOP_READ);
	
	return (((uint16_t) data[0]) << 8) + data[1];	
}

//Set the mask value in array indicated by isArray1 (where arrays have indices 1 and 0) and index char_index_in_register to targetLevel
void setIO(bool isArray1, uint8_t char_index_in_register, bool targetLevel){
	uint8_t data[2] = {0x0, 0x0};
	
	readFromAddressAndMemoryLocation(&(data[0]), 1, TCA_ADDR, CONFIG_REGISTER_0, TCA_SHOULD_STOP_READ);
	readFromAddressAndMemoryLocation(&(data[1]), 1, TCA_ADDR, CONFIG_REGISTER_1, TCA_SHOULD_STOP_READ);
	
	//left shift by character index to move target to far left, right shift by 7 to convert to 0/1 based on current position
	uint8_t currentVal = (data[isArray1] << char_index_in_register) >> 7;
	
	if(currentVal != targetLevel){
		uint8_t xorMask = targetLevel << (7-char_index_in_register);
		data[isArray1] = data[isArray1] ^ xorMask;
	}
	
	setIOMask(data[0],data[1]);
}