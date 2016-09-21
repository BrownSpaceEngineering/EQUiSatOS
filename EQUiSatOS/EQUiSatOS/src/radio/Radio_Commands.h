/*
 * Radio_Commands.h
 *
 * Created: 9/20/2016 9:50:03 PM
 *  Author: Tyler
 */ 


#ifndef RADIO_COMMANDS_H_
#define RADIO_COMMANDS_H_

#include "../processor_drivers/USART_Commands.h"

char dealer_response[4] = {1, 196, 0, 59};
char txFreq_response[4] = {1, 183, 0, 72};
char rxFreq_response[4] = {1, 185, 0, 70};
char channel_response[4] = {0x01, 0x83, 0x00, 0x7c};
char warmReset_response[4] = {0x01, 0x9d, 0x00, 0x62};
	
int working = 1;

void setCommandMode();
void setDealerMode();
void setTxFreq();
void setRxFreq();
void setChannel();
void warmReset();
void setModulationFormat();
void setLinkSpeed();
unsigned char calculateChecksum(char* data, int dataLen);
int responseCheck(char arr[]);
void initializeRadio();

#endif /* RADIO_COMMANDS_H_ */