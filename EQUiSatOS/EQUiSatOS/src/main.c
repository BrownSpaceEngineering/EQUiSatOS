//#include "runnable_configurations/flatsat.h"
#include "main.h"
#include "runnable_configurations/flatsat.h"
#include "runnable_configurations/bat_testing.h"

void The_Fall_Of_Ryan(void) {
	struct adc_module adc_instance;
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
	config_adc.positive_input = P_AI_LED1SNS;

	//setup_set_config
	adc_init(&adc_instance, 0x42004000UL, &config_adc);
	adc_enable(&adc_instance);

	if (!adc_instance.hw) {
		//You must configure the adc_instance and set it as a global variable.
		return;
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
}


void run_tests(void) {
	//flatsat_run(); //calls flatsat init
	//bat_testing_run();
	//test_equistack();
	//assert_rtos_constants();
	//assert_transmission_constants();

	//system_test(); 
	/************************************************************************/
	/* AD HOC TESTS GO AFTER HERE - Make a FUNCTION for them                */
	/************************************************************************/
	//The_Fall_Of_Ryan();

	/*configure_pwm(P_ANT_DRV1, P_ANT_DRV1_MUX);
	init_tc();
	while(get_count() < 3) {
			set_pulse_width_fraction(3, 4);
	}
	disable_pwm();
	*/
}

/************************************************************************/
/* DO NOT MODIFY BELOW HERE FOR SCRATCH TESTING - sid will come for you */
/************************************************************************/
void global_init(void) {
	// Initialize the SAM system
	system_init();

	// Get this false as fast as possible.
	/*setup_pin(true,P_LF_B2_OUTEN);
	set_output(false, P_LF_B2_OUTEN);
	setup_pin(true,P_LF_B1_OUTEN);
	set_output(false, P_LF_B1_OUTEN);
	setup_pin(true,P_L1_RUN_CHG); //TODO consider if we need these here
	set_output(false, P_L1_RUN_CHG);
	setup_pin(true,P_L2_RUN_CHG);
	set_output(false, P_L2_RUN_CHG);*/

	init_rtc();
	USART_init();
	
	init_tracelyzer();	// MUST be before anything RTOS-related! (Equistacks in init_errors!)
	
	init_errors();
}

int main(void)
{
	global_init();
	uint16_t buf[2];
	buf[0] = 0;
	buf[1] = 0;
	//system_test();
	//read_lion_volts_batch(buf);
	int x = 1; 
	//run_tests();
	run_rtos();	
}
