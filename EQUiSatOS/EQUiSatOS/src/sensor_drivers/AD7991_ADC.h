/*
 * AD7991_ADC.h
 *
 * Created: 2/19/2017 4:22:22 PM
 *  Author: BSE
 */ 


#ifndef AD7991_ADC_H_
#define AD7991_ADC_H_

#include <global.h>
#include "../processor_drivers/I2C_Commands.h"

#define AD7991_SHOULD_STOP				false
#define REGULATOR_ENABLE_WAIT_AFTER_MS		500 // ms; how long to wait after enabling regulators to read them

enum status_code AD7991_init(uint8_t board);
enum status_code AD7991_read_all(uint16_t *results, uint8_t addr);
uint16_t ad7991_adc_to_mV(uint16_t reading);
enum status_code AD7991_read_all_mV(uint16_t *results, uint8_t addr);

#endif /* AD7991_ADC_H_ */