/*
 * AD7991_ADC.h
 *
 * Created: 2/19/2017 4:22:22 PM
 *  Author: BSE
 */ 


#ifndef AD7991_ADC_H_
#define AD7991_ADC_H_

#include <asf.h>
#include "../processor_drivers/I2C_Commands.h"

#define AD7991_ADDR 0b0101000
#define AD7991_SHOULD_STOP false

void AD7991_init(void);
void AD7991_change_channel(uint8_t channel);
uint16_t AD7991_read(uint8_t channel);

#endif /* AD7991_ADC_H_ */