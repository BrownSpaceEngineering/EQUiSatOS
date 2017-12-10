/*
 * antenna_pwm.c
 *
 * Created: 12/2/17 8:58:23 PM
 *  Author: jacobleiken
 */ 

#include "antenna_pwm.h"

void pwm_test(void) {
	try_pwm_deploy(P_ANT_DRV1, P_ANT_DRV1_MUX);
	try_pwm_deploy(P_ANT_DRV2, P_ANT_DRV2_MUX);
	try_pwm_deploy(P_ANT_DRV3, P_ANT_DRV3_MUX);
}

void try_pwm_deploy(int pin, int pin_mux) {
	configure_pwm(pin, pin_mux);
	int start = get_rtc_count();
	while (get_rtc_count() < start + 3) {
		set_pulse_width_fraction(3, 4);
	}
	disable_pwm();
}