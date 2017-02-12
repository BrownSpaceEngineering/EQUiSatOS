/*
 * LTC1380_Multiplexer_Commands.c
 *
 * Created: 2/12/2017 3:51:02 PM
 *  Author: BSE
 */ 

#include "LTC1380_Multiplexer_Commands.h"

void LTC1380_init(void){
	//nothing yet
};


void LTC1380_channel_select(uint8_t target){
	if(target > 7){
		return;
	}
	
	//add EN bit of 1 to position 4 with value 2^4
	uint8_t bufferResult = target + 8;
	
	writeDataToAddress(&bufferResult,1,MULTIPLEXER_I2C,true);
}

void LTC1380_disable(void){
	uint8_t off = 0;
	
	writeDataToAddress(&off,1,MULTIPLEXER_I2C,true);
}