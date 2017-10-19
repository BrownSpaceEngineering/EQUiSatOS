/*
 * Radio_Commands.h
 *
 * Created: 9/20/2016 9:50:03 PM
 *  Author: Tyler
 */ 


#ifndef RADIO_COMMANDS_H_
#define RADIO_COMMANDS_H_

#include "../processor_drivers/USART_Commands.h"

void setCommandMode(void);
void setDealerMode(void);
void setTxFreq(void);
void setRxFreq(void);
void setChannel(void);
void warmReset(void);
void setModulationFormat(void);
void setLinkSpeed(void);
unsigned char calculateChecksum(char* data, int dataLen);
int responseCheck(char arr[]);
void initializeRadio(void);
void setSendEnable(bool level);
void setReceiveEnable(bool level);

#endif /* RADIO_COMMANDS_H_ */