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


void LTC1380_channel_select(uint8_t addr, uint8_t target, return_struct_8 rs){
	if(target > 7){
		return;
	}
	
	//add EN bit of 1 to position 4 with value 2^4
	uint8_t bufferResult = target + 8;
	rs.return_status = writeDataToAddress(&bufferResult,1,addr,true);
	rs.return_value = bufferResult;
	
	return rs;
}

void LTC1380_disable(uint8_t addr, return_struct_8 rs){
	uint8_t off = 0;
	rs.return_status = writeDataToAddress(&off,1,addr,true);
	rs.return_value = off;
	
	return rs;
}
