/*
 * Radio_Commands.h
 *
 * Created: 9/20/2016 9:50:03 PM
 *  Author: Tyler
 */ 


#ifndef RADIO_COMMANDS_H_
#define RADIO_COMMANDS_H_

#include "../processor_drivers/USART_Commands.h"

void set_command_mode(void);
void XDL_init(void);
uint16_t XDL_get_temperature(void);
void warm_reset(void);

void setTXEnable(bool enable);
void setRXEnable(bool enable);
void set3V6Power(bool on);
void setRadioPower(bool on);

#endif /* RADIO_COMMANDS_H_ */