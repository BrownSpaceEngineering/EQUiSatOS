/*
 * antenna_pwm.c
 *
 * Created: 12/2/17 8:58:23 PM
 *  Author: jacobleiken
 */ 

#include "antenna_pwm.h"

void pwm_test(void) {
	try_pwm_deploy(P_ANT_DRV1, P_ANT_DRV1_MUX, DEFAULT_PWM_MS_RUN);
	try_pwm_deploy(P_ANT_DRV2, P_ANT_DRV2_MUX, DEFAULT_PWM_MS_RUN);
	try_pwm_deploy(P_ANT_DRV3, P_ANT_DRV3_MUX, DEFAULT_PWM_MS_RUN);
}

void try_pwm_deploy(int pin, int pin_mux, int ms) {
	configure_pwm(pin, pin_mux);
	set_pulse_width_fraction(3, 4);
	delay_ms(ms);
	//vTaskDelay(ms);
	disable_pwm();
}