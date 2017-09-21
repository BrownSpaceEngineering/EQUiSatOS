/*
 * PWM_COMMANDS.c
 *
 * Created: 12/4/2016 3:55:13 PM
 *  Author: Daniel
 */ 

#include "PWM_Commands.h"

void configure_pwm(int pwm_pin, int pwm_mux) {
	struct tcc_config config_tcc;
	tcc_get_config_defaults(&config_tcc, CONF_PWM_MODULE);
	
	config_tcc.counter.clock_prescaler = TCC_CLOCK_PRESCALER_DIV1; //clock prescaler
	config_tcc.counter.period = PWM_PERIOD; //maximum number of counter
	config_tcc.compare.wave_generation = TCC_WAVE_GENERATION_SINGLE_SLOPE_PWM;
	config_tcc.compare.match[CONF_PWM_CHANNEL] = PWM_ON_PERIOD; //when to change output
	
	config_tcc.pins.enable_wave_out_pin[CONF_PWM_OUTPUT] = true;
	config_tcc.pins.wave_out_pin[CONF_PWM_OUTPUT]        = pwm_pin;
	config_tcc.pins.wave_out_pin_mux[CONF_PWM_OUTPUT]    = pwm_mux;

	tcc_init(&tcc_instance, CONF_PWM_MODULE, &config_tcc);

	tcc_enable(&tcc_instance);
}

/*
	Set 
*/
bool set_pulse_width_fraction(int numerator, int denominator) {
	if(numerator > denominator){
		return false;
	}else{
		int toSet = (PWM_PERIOD*numerator)/denominator;
		tcc_set_compare_value(&tcc_instance,CONF_PWM_CHANNEL,toSet);
		return true;
	}
}
