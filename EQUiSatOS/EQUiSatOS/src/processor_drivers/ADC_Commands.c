/*
 * CFile1.c
 *
 * Created: 5/8/2015 3:20:16 PM
 *  Author: Daniel
 */ 

#include "ADC_Commands.h"
void configure_adc(struct adc_module *adc_instance, enum adc_positive_input pin) {
	struct adc_config config_adc;
	adc_get_config_defaults(&config_adc);

	config_adc.resolution = ADC_RESOLUTION_10BIT;
	config_adc.correction.correction_enable = true;
	if (pin == P_AI_TEMP_OUT){
		config_adc.correction.offset_correction = 22;
		config_adc.correction.gain_correction = 2920; // 2048 = 1x
	}else{
		config_adc.correction.offset_correction = 48;
		config_adc.correction.gain_correction = 2220; // 2048 = 1x
	}
	
	config_adc.correction.gain_correction = 2220; // 2048 = 1x
	config_adc.clock_prescaler = ADC_CLOCK_PRESCALER_DIV4;
	config_adc.reference = ADC_REFERENCE_INTVCC0; //VCC/1.48
	
	//Maybe add this or similar things for other pins
	/*if (pin == P_AI_LED1SNS || pin==P_AI_LED2SNS || pin == P_AI_LED3SNS || pin==P_AI_LED4SNS){
		config_adc.gain_factor = ADC_GAIN_FACTOR_16X;
		config_adc.reference = ADC_REFERENCE_INT1V;
	}*/
	
	//Hardware averaging - couldn't get this to work
	//config_adc.accumulate_samples = ADC_ACCUMULATE_SAMPLES_1024;
	//config_adc.divide_result = ADC_DIVIDE_RESULT_16;
	
	//Set the pin
	config_adc.positive_input = pin;
	
	//setup_set_config
	adc_init(adc_instance, ADC, &config_adc);
	adc_enable(adc_instance);
}

//reads the current voltage from the ADC connection
// Currently reads a 10 bit value
uint16_t read_adc(struct adc_module adc_instance) {
	if (!adc_instance.hw) {
		//You must configure the adc_instance and set it as a global variable.
		return -1;
	}
	
	uint16_t result;
	int status;
	
	adc_start_conversion(&adc_instance);
	
	//uint8_t scale = 218;//3300/1.48/1024.0; //3.3V/1.48 reference, 2^10 range
	
	do {
		// Wait for conversion to be done and read out result
		status = adc_read(&adc_instance, &result);
	} while (status == STATUS_BUSY);
	
	adc_disable(&adc_instance);
	return result;
}

uint8_t convert_ir_to_8_bit(uint16_t input) {
	bool addOne = (input & 0b0000000000000011) >= 2;
	uint8_t _8bitResult = input >> 2;
	if (addOne) {
		_8bitResult = _8bitResult+1;
	}
	return _8bitResult;
}

 // Given an ADC channel, reads from ADC with <num_avg> software averaging
 uint16_t readFromADC(enum adc_positive_input pin, int num_avg){
	 struct adc_module adc_instance;
	 configure_adc(&adc_instance,pin);
	 //read_adc(adc_instance);
	 //adc_enable(&adc_instance);
	 uint16_t sum = 0;
	 
	 for (int i=0; i<num_avg; i++){
		 adc_enable(&adc_instance);
		 sum = sum +read_adc(adc_instance);
	 }
	 return sum/num_avg;
 }

//Converts from 10bit ADC reading to float voltage
//Todo - remove this because no floats for real sat
float convertToVoltage(uint16_t reading){
	return (((float) (reading))/1024*3.300/1.48); //converts from 10 bit to V
}