/*
 * AD7991_ADC.c
 *
 * Created: 2/19/2017 4:32:43 PM
 *  Author: BSE
 */ 

#include "AD7991_ADC.h"

//Make sure I2C is configured (on SERCOM4) 
enum status_code AD7991_init(){	
	//(1111 0000) write to AD7991 address to read from all regulators 
	//read 8 bytes 1-2 -> channel 1, 3-4 -> channel 2 ... etc 
	uint8_t all_reg = 0xf0;
	
	return writeDataToAddress(all_reg, 1, AD7991_ADDR, true);	
}

enum status_code AD7991_read_all(uint8_t *results){
	//AD7991_change_channel(channel);
	uint8_t buffer[8];// = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
	struct return_struct_16 rs;
	enum status_code read = readFromAddress(buffer,8,AD7991_ADDR,true);
	results[0] = ((buffer[0] & 0b00001111) << 8) + buffer[1];
	results[1] = ((buffer[2] & 0b00001111) << 8) + buffer[3];
	results[2] = ((buffer[4] & 0b00001111) << 8) + buffer[5];
	results[3] = ((buffer[6] & 0b00001111) << 8) + buffer[7];
	
	return read; 

};


/////////////////////////////////////////////////////////////////////////////
/* ONLY USE FUNCTIONS ABOVE THIS LINE *//////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

//change the channel of the external ADC
void AD7991_change_channel(uint8_t channel){
	uint8_t target;//[] = {0b10000};
	
	//int target[] = {0x0};
	switch(channel){
		case 0x0 :
			target = 0x1 << 0x4;
			break;
		case 0x1 :
			target = 0x1 << 0x5;
			break;
		case 0x2 :
			target = 0x1 << 0x6;
			break;
		case 0x3 :
			target = 0x1 << 0x7;
			break;
		default:
			//this is not a valid channel
			return;
			
	}
	//target = target + 3; //Disables the sample delay and bit trial mechanisms
	struct return_struct_0 rs;
	rs.return_status = writeDataToAddress(target, 1, AD7991_ADDR, true);
	
	int a = 10;
	//return rs;
}

uint16_t AD7991_read(uint8_t channel){
	//AD7991_change_channel(channel);
	uint8_t buffer[] = {0x0, 0x0};
	struct return_struct_16 rs;
	rs.return_status = readFromAddress(buffer,2,AD7991_ADDR,false);
	rs.return_value =  ((buffer[0] & 0b00001111) << 8)+ buffer[1];


	int a = 5; 
	return rs.return_value;
};



