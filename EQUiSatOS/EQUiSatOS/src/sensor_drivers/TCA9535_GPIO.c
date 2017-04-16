/*
 * TCA9535_GPIO.c
 *
 * Created: 3/15/2017 9:17:59 PM
 *  Author: BSE
 */ 

#include "TCA9535_GPIO.h"

enum status_code TCA9535_init(void){
	uint16_t x = readTCA9535Levels().return_value;
	return setIOMask(IOMask0,IOMask1);
}

//Set the mask for the configuration register
enum status_code setIOMask(uint8_t reg0, uint8_t reg1){
	uint8_t data0[2] = {CONFIG_REGISTER_0, reg0};
	uint8_t data1[2] = {CONFIG_REGISTER_1, reg1};
	
	enum status_code statc = writeDataToAddress(data0, 2, TCA_ADDR, TCA_SHOULD_STOP_WRITE);
	if (statc & 0xf0 != 0) {
		return statc;
	}
	return writeDataToAddress(data1, 2, TCA_ADDR, TCA_SHOULD_STOP_WRITE);
}

return_struct_16 readTCA9535Levels(void){
	uint8_t data[2] = {0x0, 0x0};
	
	enum status_code statc1 = readFromAddressAndMemoryLocation(&(data[0]), 1, TCA_ADDR, INPUTS_REGISTER_0, TCA_SHOULD_STOP_READ);
	if (statc1 & 0xf0 != 0) {
		struct return_struct_16 rs;
		rs.return_status = statc1;
		// PLACEHOLDER
		rs.return_value = -1;
		return rs;
	}
	enum status_code statc2 = readFromAddressAndMemoryLocation(&(data[1]), 1, TCA_ADDR, INPUTS_REGISTER_1, TCA_SHOULD_STOP_READ);
	
	struct return_struct_16 rs;
	rs.return_status = statc2;
	rs.return_value = (((uint16_t) data[0]) << 8) + data[1];
	return rs;	
}

//Set the mask value in array indicated by isArray1 (where arrays have indices 1 and 0) and index char_index_in_register to targetLevel
enum status_code setIO(bool isArray1, uint8_t char_index_in_register, bool targetLevel){
	uint8_t data[2] = {0x0, 0x0};
	
	enum status_code statc1 = readFromAddressAndMemoryLocation(&(data[0]), 1, TCA_ADDR, CONFIG_REGISTER_0, TCA_SHOULD_STOP_READ);
	if (statc1 & 0xf0 != 0) {
		return statc1;
	}
	enum status_code statc2 = readFromAddressAndMemoryLocation(&(data[1]), 1, TCA_ADDR, CONFIG_REGISTER_1, TCA_SHOULD_STOP_READ);
	if (statc2 & 0xf0 != 0) {
		return statc2;
	}
	
	//left shift by character index to move target to far left, right shift by 7 to convert to 0/1 based on current position
	uint8_t currentVal = (data[isArray1] << char_index_in_register) >> 7;
	
	if(currentVal != targetLevel){
		uint8_t xorMask = targetLevel << (7-char_index_in_register);
		data[isArray1] = data[isArray1] ^ xorMask;
	}
	
	return setIOMask(data[0],data[1]);
}