/*
 * antenna_pwm.c
 *
 * Created: 12/2/17 8:58:23 PM
 *  Author: jacobleiken
 */ 

#include "antenna_pwm.h"

static int curren_pwm_pin = 1;
static int current_on_cycle = PWM_PERIOD / 2;

// not for flight
static void try_pwm_deploy_basic(int pin, int pin_mux, int ms, int p_ant) {
	for (int i = 8; i < 16; i++) {
		configure_pwm(pin, pin_mux, p_ant);
		enable_pwm(i);
		delay_ms(ms);
		disable_pwm();
	}
}

void pwm_test(void) {
	try_pwm_deploy_basic(P_ANT_DRV1, P_ANT_DRV1_MUX, PWM_LENGTH_MS, 1);
	//try_pwm_deploy_basic(P_ANT_DRV2, P_ANT_DRV2_MUX, PWM_LENGTH_MS, 2);
	//try_pwm_deploy_basic(P_ANT_DRV3, P_ANT_DRV3_MUX, PWM_LENGTH_MS, 3);
}

void try_pwm_deploy(int pin, int pin_mux, int ms, int p_ant) {
	#ifdef ANTENNA_DEPLOY_ACTIVE
		configure_pwm(pin, pin_mux, p_ant);
	
		//hardware_state_mutex_take();
		enable_pwm(current_on_cycle);
		//get_hw_states()->antenna_deploying = true;
		//hardware_state_mutex_give();
	
		delay_ms(ms);
		//vTaskDelay(ms);
	
		//hardware_state_mutex_take();
		disable_pwm();
		bool can_cont = true;
		if (p_ant == 1) {
			lion_current_batch li;
			panelref_lref_batch toss;
			read_ad7991_batbrd(li, toss);
			if (li[0] > PWM_MAX_CUR || li[1] > PWM_MAX_CUR) {
				can_cont = false;
			}
		} else {
			lifepo_current_batch lf;
			//read_lifepo_current_batch(lf);
			if (lf[0] > PWM_MAX_CUR || lf[1] > PWM_MAX_CUR) {
				can_cont = false;
			}
		}
		if (can_cont) {
			// increment for next call
			current_on_cycle++;
			// it can't be on always (16 /16 == 1), so if it's at 16 switch to the next pin
			if (current_on_cycle >= PWM_PERIOD) {
				current_on_cycle = PWM_PERIOD / 2;
				curren_pwm_pin++;
				// now if the pin is past 3, set it back to 1
				if (curren_pwm_pin > 3) {
					curren_pwm_pin = 1;
				}
			}
		} else {
			curren_pwm_pin++;
			// now if the pin is past 3, set it back to 1
			if (curren_pwm_pin > 3) {
				curren_pwm_pin = 1;
			}
		}
		//get_hw_states()->antenna_deploying = false;
		//hardware_state_mutex_give();
	
	#endif
}

int get_current_pwm_pin(void) {
	return curren_pwm_pin;
}