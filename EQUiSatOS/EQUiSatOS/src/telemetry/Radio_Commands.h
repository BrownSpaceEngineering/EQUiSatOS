/*
 * Radio_Commands.h
 *
 * Created: 9/20/2016 9:50:03 PM
 *  Author: Tyler
 */ 


#ifndef RADIO_COMMANDS_H_
#define RADIO_COMMANDS_H_

#include "../processor_drivers/USART_Commands.h"
#include "../sensor_drivers/sensor_read_commands.h"

void set_command_mode(void);
uint16_t XDL_get_temperature(void);
void warm_reset(void);

void radio_init(void);
void setRadioState(bool enable);
void setTXEnable(bool enable);
void setRXEnable(bool enable);
void set3V6Power(bool on);
void setRadioPower(bool on);

#endif /* RADIO_COMMANDS_H_ */