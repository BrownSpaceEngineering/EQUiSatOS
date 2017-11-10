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

enum status_code read_adc(struct adc_module adc_instance, uint16_t* buf);
enum status_code configure_adc(struct adc_module *adc_instance, enum adc_positive_input pin);
uint8_t convert_ir_to_8_bit(uint16_t input);
enum status_code readFromADC(enum adc_positive_input pin, int num_avg, uint16_t* buf);
float convertToVoltage(uint16_t reading);

#endif /* ADC_COMMANDS_H */