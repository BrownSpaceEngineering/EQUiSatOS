/*
 * PWM_COMMANDS.c
 *
 * Created: 12/4/2016 3:55:13 PM
 *  Author: Daniel
 */ 

#include "PWM_Commands.h"

void configure_pwm(int pwm_pin, int pwm_mux, int p_ant) {
	#ifdef ANTENNA_DEPLOY_ACTIVE
		struct tcc_config config_tcc;
		tcc_get_config_defaults(&config_tcc, CONF_PWM_MODULE);
	
		config_tcc.counter.clock_prescaler = TCC_CLOCK_PRESCALER_DIV1; //clock prescaler
		config_tcc.counter.period = PWM_PERIOD; //maximum number of counter
		config_tcc.compare.wave_generation = TCC_WAVE_GENERATION_SINGLE_SLOPE_PWM;
		int wo;
		if (p_ant == 1) {
			channel = 3;
			wo = 7;
		} else if (p_ant == 2) {
			channel = 0;
			wo = 4;
		} else if (p_ant == 3) {
			channel = 1;
			wo = 5;
		}
		config_tcc.compare.match[channel] = PWM_ON_PERIOD; //when to change output
	
		config_tcc.pins.enable_wave_out_pin[wo] = true;
		config_tcc.pins.wave_out_pin[wo]        = pwm_pin;
		config_tcc.pins.wave_out_pin_mux[wo]    = pwm_mux;

		tcc_init(&tcc_instance, CONF_PWM_MODULE, &config_tcc);

		tcc_enable(&tcc_instance);
	#endif
}

// on_fraction will specify the duty cycle of on_fraction / PWM_PERIOD
void enable_pwm(int on_fraction) {
	#ifdef ANTENNA_DEPLOY_ACTIVE
		if (on_fraction < PWM_PERIOD) {
			tcc_set_compare_value(&tcc_instance, channel, on_fraction);
		}
	#endif
}

void disable_pwm(void) {
	#ifdef ANTENNA_DEPLOY_ACTIVE
		tcc_disable(&tcc_instance);
		tcc_reset(&tcc_instance);
	#endif
}
