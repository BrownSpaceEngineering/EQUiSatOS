#include "ADC_Commands.h"

enum status_code configure_adc(struct adc_module *adc_instance, enum adc_positive_input pin, bool precise) {
	#ifndef XPLAINED
	struct adc_config config_adc;
	adc_get_config_defaults(&config_adc);

	config_adc.resolution = ADC_RESOLUTION_CUSTOM;
	config_adc.correction.correction_enable = true;

	config_adc.correction.offset_correction = 10;
	config_adc.correction.gain_correction = 2049; // 2048 = 1x


	//config_adc.clock_prescaler = ADC_CLOCK_PRESCALER_DIV4;
	config_adc.reference = ADC_REFERENCE_INTVCC0; //VCC/1.48

	//Maybe add this or similar things for other pins
	/*if (pin == P_AI_LED1SNS || pin==P_AI_LED2SNS || pin == P_AI_LED3SNS || pin==P_AI_LED4SNS){
		config_adc.gain_factor = ADC_GAIN_FACTOR_16X;
		config_adc.reference = ADC_REFERENCE_INT1V;
	}*/

	// TODO: See how high we can go for samples without slowing things down too much
	config_adc.accumulate_samples = precise ? ADC_ACCUMULATE_SAMPLES_1024 : ADC_ACCUMULATE_SAMPLES_64;
	config_adc.divide_result = ADC_DIVIDE_RESULT_16;

	//Set the pin
	config_adc.positive_input = pin;

	//setup_set_config
	enum status_code sc = adc_init(adc_instance, ADC, &config_adc);
	if (is_error(sc)) {
		return sc;
	} else {
		return adc_enable(adc_instance);
	}
	
	#else 
		return STATUS_OK;
	#endif
}

//reads the current voltage from the ADC connection (with hardware averaging) into a 16 bit buffer
enum status_code read_adc(struct adc_module adc_instance, uint16_t* buf) {
	#ifndef XPLAINED
	if (!adc_instance.hw) {
		//You must configure the adc_instance and set it as a global variable.
		return -1;
	}

	enum status_code status;
	adc_start_conversion(&adc_instance);
	//uint8_t scale = 218;//3300/1.48/1024.0; //3.3V/1.48 reference, 2^10 range

	do {
		// Wait for conversion to be done and read out result
		status = adc_read(&adc_instance, buf);
		if (is_error(status)) {
			return status;
		}
	} while (status == STATUS_BUSY);

	return adc_disable(&adc_instance);
	
	#else
		uint16_t xplained_buf = ADC_RD_VAL_XPLAINED;
		memcpy(buf, &xplained_buf, 2);
		return STATUS_OK;
	#endif
}

//Converts from 16bit ADC reading to mV
uint16_t convert_adc_to_mV(uint16_t reading){
	return (reading*1000*330/4095/148);
}

enum status_code read_adc_mV(struct adc_module adc_instance, uint16_t* buf) {
	enum status_code status = read_adc(adc_instance, buf);
	*buf = convert_adc_to_mV(*buf);
	return status;
}
