/*
 * TEMD6200_commands.h
 *
 * Created: 2/3/2016 8:30:26 PM
 *  Author: Tyler
 */ 


#ifndef TEMD6200_COMMANDS_H_
#define TEMD6200_COMMANDS_H_

#include "../processor_drivers/ADC_Commands.h"

#define TEMD6200_VOLTAGE_LIMIT_MV 3000

float brightness(struct adc_module adc_reader);



#endif /* TEMD6200_COMMANDS_H_ */