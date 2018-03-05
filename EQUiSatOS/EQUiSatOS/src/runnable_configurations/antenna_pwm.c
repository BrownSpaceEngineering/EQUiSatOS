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
	try_pwm_deploy(P_ANT_DRV1, P_ANT_DRV1_MUX, PWM_LENGTH_MS, 1);
	try_pwm_deploy_basic(P_ANT_DRV2, P_ANT_DRV2_MUX, PWM_LENGTH_MS, 2);
	try_pwm_deploy_basic(P_ANT_DRV3, P_ANT_DRV3_MUX, PWM_LENGTH_MS, 3);
}

void try_pwm_deploy(long pin, long pin_mux, int ms, uint8_t p_ant) {
	#ifdef ANTENNA_DEPLOY_ACTIVE
		configure_pwm(pin, pin_mux, p_ant);
	
		hardware_state_mutex_take();
		enable_pwm(current_on_cycle);
		get_hw_states()->antenna_deploying = true;
		hardware_state_mutex_give();
		
		// read current (both just in case) so we can shut it down if we need
		lion_current_batch li;
		panelref_lref_batch toss;
		read_ad7991_batbrd(li, toss);
		lifepo_current_batch lf;
		read_lifepo_current_batch(lf, false);
	
		//delay_ms(ms); // for testing only
		vTaskDelay(ms);
	
		hardware_state_mutex_take();
		disable_pwm();
		get_hw_states()->antenna_deploying = false;
		hardware_state_mutex_give();
		
		bool can_cont = true;
		if (p_ant == 1) {
			uint16_t li1 = (uint16_t)li[0] << 8;
			uint16_t li2 = (uint16_t)li[1] << 8;
			print("PWM was on LiON\nCurrent on 1: %d\nCurrent on 2: %d\n", li1, li2);
			if (li1 > PWM_MAX_CUR || li2 > PWM_MAX_CUR) {
				can_cont = false;
			}
		} else {
			uint16_t bank1 = (uint16_t)(lf[0] + lf[1]) << 8;
			uint16_t bank2 = (uint16_t)(lf[2] + lf[3]) << 8;
			print("PWM was on LiFePO4\nCurrent on bank 1: %d\nCurrent on bank 2: %d\n", bank1, bank2);
			if (bank1 > PWM_MAX_CUR || bank2 > PWM_MAX_CUR) {
				can_cont = false;
			}
		}
		if (can_cont) {
			// increment for next call
			current_on_cycle++;
			// it shouldn't be on too much, so if it's at 14 switch to the next pin
			if (current_on_cycle >= (PWM_PERIOD - 2)) {
				current_on_cycle = PWM_PERIOD / 2;
				curren_pwm_pin++;
				// now if the pin is past 3, set it back to 1
				if (curren_pwm_pin > 3) {
					curren_pwm_pin = 1;
				}
			}
		} else {
			current_on_cycle = PWM_PERIOD / 2;
			curren_pwm_pin++;
			// now if the pin is past 3, set it back to 1
			if (curren_pwm_pin > 3) {
				curren_pwm_pin = 1;
			}
		}
	#endif
}

int get_current_pwm_pin(void) {
	return curren_pwm_pin;
}