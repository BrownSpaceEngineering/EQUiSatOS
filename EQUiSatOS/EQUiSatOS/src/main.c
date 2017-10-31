//#include "runnable_configurations/flatsat.h"
#include "main.h"
#include "runnable_configurations/flatsat.h"
#include "runnable_configurations/bat_testing.h"

int main(void)
{
	// Initialize the SAM system
	system_init();

	//Get this false as fast as possible.
	setup_pin(true,P_LF_B2_OUTEN);
	set_output(false, P_LF_B2_OUTEN); 
	setup_pin(true,P_LF_B1_OUTEN
	set_output(false, P_LF_B1_OUTEN);
	
	//init_rtc();
	USART_init();
	
	
	/*struct adc_module *adc_instance;
	struct adc_config config_adc;
	adc_get_config_defaults(&config_adc);

	config_adc.resolution = ADC_RESOLUTION_10BIT;
	config_adc.correction.correction_enable = true;
	if (ADC_POSITIVE_INPUT_PIN19 == P_AI_TEMP_OUT){
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
	/*
	config_adc.positive_input = ADC_POSITIVE_INPUT_PIN19;
	
	//setup_set_config
	adc_init(adc_instance, ADC, &config_adc);
	adc_enable(adc_instance);
	
	if (!&adc_instance || !adc_instance->hw) {
		//You must configure the adc_instance and set it as a global variable.
		return -1;
	}
	
	uint16_t result;
	int status;
	
	adc_start_conversion(adc_instance);
	
	uint8_t scale = 218;//3300/1.48/1024.0; //3.3V/1.48 reference, 2^10 range
	
	do {
		// Wait for conversion to be done and read out result
		status = adc_read(adc_instance, &result);
	} while (status == STATUS_BUSY);
	
	adc_disable(adc_instance);*/
	
	//flatsat_run(); //calls flatsat init
	
	//init_errors(); // init error stack? (TODO: do this here or start of RTOS?)
	
	bat_testing_run();
	
	
	//run_rtos();
	
	/* TESTS */
	/*configure_pwm(P_ANT_DRV1, P_ANT_DRV1_MUX);
	init_rtc();
	while(get_count() < 3) {
			set_pulse_width_fraction(3, 4);		
	}*/
	
	//disable_pwm();		
	//test_equistack();
	//assert_rtos_constants();
	//assert_transmission_constants();
}
