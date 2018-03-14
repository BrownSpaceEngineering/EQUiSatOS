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
enum status_code AD7991_init(uint8_t board){
	return writeDataToAddress(&all_adc_reg, 1, board, true);
}

// Reads from all 4 channels of the chip, each num_samples times and averages them
//results should be of length 4
enum status_code AD7991_read_all(uint16_t *results, uint8_t addr){
	// Initialize
	uint8_t num_samples = 4;
	uint8_t buffer[8*num_samples];
	uint16_t results0, results1, results2, results3;
	results0 = 0;
	results1 = 0;
	results2 = 0;
	results3 = 0;
	// I2C transaction
	enum status_code read = readFromAddress(buffer,8*num_samples,addr,false);
	
	//Software averaging
	for (int i=0; i<num_samples; i++){
		results0 = results0 + (((buffer[(8*i)] & 0b00001111) << 8) + buffer[(8*i+1)]);
		results1 = results1 + (((buffer[(8*i+2)] & 0b00001111) << 8) + buffer[(8*i+3)]);
		results2 = results2 + (((buffer[(8*i+4)] & 0b00001111) << 8) + buffer[(8*i+5)]);
		results3 = results3 + (((buffer[(8*i+6)] & 0b00001111) << 8) + buffer[(8*i+7)]);
	}	
	results[0] = results0/num_samples;
	results[1] = results1/num_samples;
	results[2] = results2/num_samples;
	results[3] = results3/num_samples;
	
	return read;
}

uint16_t ad7991_adc_to_mV(uint16_t reading) {
	return reading*1000*33/4096/10;
}

enum status_code AD7991_read_all_mV(uint16_t *results, uint8_t addr) {
	enum status_code status = AD7991_read_all(results, addr);
	for (int i = 0; i < 4; i++) {
		results[i] = ad7991_adc_to_mV(results[i]);
	}
	return status;
}