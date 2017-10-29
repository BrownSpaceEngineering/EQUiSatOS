/*
 * CFile1.c
 *
 * Created: 5/8/2015 3:20:16 PM
 *  Author: Daniel
 */ 

#include "ADC_Commands.h"
void configure_adc(struct adc_module *adc_instance, enum adc_positive_input pin) {
	struct adc_config config_adc;
	// setup_config_defaults
	adc_get_config_defaults(&config_adc);

	config_adc.resolution = ADC_RESOLUTION_10BIT;

	config_adc.positive_input = pin;
	//setup_set_config
	adc_init(adc_instance, ADC, &config_adc);
	adc_enable(adc_instance);
}

//reads the current voltage from the ADC connection
float readVoltagemV(struct adc_module adc_instance) {
	if (!&adc_instance || !adc_instance.hw) {
		//You must configure the adc_instance and set it as a global variable.
		return -1;
	}
	
	uint16_t result = 0;
	int status;
	
	//start conversion
	adc_start_conversion(&adc_instance);
	
	do {
		// Wait for conversion to be done and read out result
		status = adc_read(&adc_instance, &result);
	} while (status == STATUS_BUSY);
	float resFloat = result;
	return resFloat;
}

float readVoltageV(struct adc_module adc_instance){
	return readVoltagemV(adc_instance)/1000.0;
} 

uint16_t read_adc(struct adc_module adc_instance) {
	return 0;
}
