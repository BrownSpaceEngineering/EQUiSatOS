/*
 * IncFile1.h
 *
 * Created: 5/8/2015 3:20:25 PM
 *  Author: Daniel
 */ 


#ifndef ADC_COMMANDS_H
#define ADC_COMMANDS_H

#include <global.h>
//#include "conf_uart_serial.h"
#include <inttypes.h>

uint16_t readVoltagemV(struct adc_module adc_instance);
float readVoltageV(struct adc_module adc_instance);
float resultToVoltage(float result);
void configure_adc(struct adc_module *adc_instance, enum adc_positive_input pin);
uint8_t convert_ir_to_8_bit(uint16_t input);

#endif /* ADC_COMMANDS_H */