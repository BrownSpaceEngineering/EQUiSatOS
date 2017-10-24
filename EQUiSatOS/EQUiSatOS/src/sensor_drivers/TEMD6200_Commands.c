/*
 * TEMD21_Commands.c
 *
 * Created: 2/3/2016 8:29:56 PM
 *  Author: Tyler
 */ 

#include "TEMD6200_Commands.h"

float brightness(struct adc_module adc_reader){
	float voltage = read_adc(adc_reader);
	return voltage;
}
