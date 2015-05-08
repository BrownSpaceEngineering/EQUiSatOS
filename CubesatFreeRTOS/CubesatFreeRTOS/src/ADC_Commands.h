/*
 * IncFile1.h
 *
 * Created: 5/8/2015 3:20:25 PM
 *  Author: Daniel
 */ 


#ifndef ADC_COMMANDS_H
#define ADC_COMMANDS_H

#include <asf.h>
#include "conf_uart_serial.h"
#include <inttypes.h>
#include <delay.h>

float readVoltagemV(struct adc_module adc_instance);
float readVoltageV(struct adc_module adc_instance);
float resultToVoltage(float result);
float voltageToTemp(float voltage);
void configure_adc(void);

#endif /* ADC_COMMANDS_H */