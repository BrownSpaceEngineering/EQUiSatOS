/*
 * AD7991_ADC.c
 *
 * Created: 2/19/2017 4:32:43 PM
 *  Author: BSE
 */ 

#include "AD7991_ADC.h"

void AD7991_init(){
	//AD7991_change_channel(0x0);	
}

//change the channel of the external ADC
void AD7991_change_channel(uint8_t channel){
	uint8_t target[] = {0b10000000};
	/*
	int target[] = {0x0};
	switch(channel){
		case 0x0 :
			target[0] = 0x1 << 0x4;
			break;
		case 0x1 :
			target[0] = 0x1 << 0x5;
			break;
		case 0x2 :
			target[0] = 0x1 << 0x6;
			break;
		case 0x3 :
			target[0] = 0x1 << 0x7;
			break;
		default:
			//this is not a valid channel
			return;
			
	}*/
	
	writeDataToAddress(target, 1, AD7991_ADDR, true);
}
uint16_t AD7991_read(uint8_t channel){
	//AD7991_change_channel(channel);
	uint8_t buffer[] = {0x0, 0x0};
	readFromAddress(buffer,2,AD7991_ADDR,true);
	uint16_t toReturn = ((buffer[0] & 0b00001111) << 8) + buffer[1];

	return toReturn;
};
