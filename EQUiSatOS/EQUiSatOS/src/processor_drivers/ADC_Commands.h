/*
 * IncFile1.h
 *
 * Created: 5/8/2015 3:20:25 PM
 *  Author: Daniel
 */


#ifndef ADC_COMMANDS_H
#define ADC_COMMANDS_H

#include <global.h>
#include <inttypes.h>

enum status_code read_adc(struct adc_module adc_instance, uint16_t* buf);
enum status_code configure_adc(struct adc_module *adc_instance, enum adc_positive_input pin, bool precise);
uint16_t convert_adc_to_mV(uint16_t reading);
enum status_code read_adc_mV(struct adc_module adc_instance, uint16_t* buf);

#endif /* ADC_COMMANDS_H */
