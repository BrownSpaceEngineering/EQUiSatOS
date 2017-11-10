/*
 * LTC1380_Multiplexer_Commands.c
 *
 * Created: 2/12/2017 3:51:02 PM
 *  Author: BSE
 */ 

#include "LTC1380_Multiplexer_Commands.h"

void LTC1380_init(void){
	//nothing yet
}


enum status_code LTC1380_channel_select(uint8_t addr, uint8_t target, uint8_t* rs){
	if(target > 7){
		return -1;
	}
	
	//add EN bit of 1 to position 4 with value 2^4
	uint8_t bufferResult = target + 8;
	enum status_code sc = writeDataToAddress(&bufferResult,1,addr,true);
	*rs = bufferResult;
	return sc;
}

enum status_code LTC1380_disable(uint8_t addr, uint8_t* rs) {
	return writeDataToAddress(rs,1,addr,true);
}
