/*
 * AD7991_ADC.c
 *
 * Created: 2/19/2017 4:32:43 PM
 *  Author: BSE
 */ 

#include "AD7991_ADC.h"

//(1111 0000) write to AD7991 address to read from all regulators
//read 8 bytes 1-2 -> channel 1, 3-4 -> channel 2 ... etc
uint8_t all_adc_reg = 0xf0;

// Make sure I2C is configured (on SERCOM4) 
enum status_code AD7991_init(){	
	// Initializes battery board
	//enum status_code code0 = writeDataToAddress(&all_adc_reg, 1, AD7991_ADDR_0, true);
	// Initializes control board
	enum status_code code1 = writeDataToAddress(&all_adc_reg, 1, AD7991_ADDR_1, true);

	
	return code1; 
}

enum status_code AD7991_read_all(uint16_t *results, uint8_t addr){
	uint8_t buffer[8];
	enum status_code read = readFromAddress(buffer,8,addr,false);
	results[0] = ((buffer[0] & 0b00001111) << 8) + buffer[1];
	results[1] = ((buffer[2] & 0b00001111) << 8) + buffer[3];
	results[2] = ((buffer[4] & 0b00001111) << 8) + buffer[5];
	results[3] = ((buffer[6] & 0b00001111) << 8) + buffer[7];
	
	return read;
}

