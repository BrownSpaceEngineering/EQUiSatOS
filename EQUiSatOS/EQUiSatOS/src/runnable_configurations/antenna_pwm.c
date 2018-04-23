/*
 * antenna_pwm.c
 *
 * Created: 12/2/17 8:58:23 PM
 *  Author: jacobleiken
 */

#include "antenna_pwm.h"

static int curren_pwm_pin = 1;
static int current_on_cycle = PWM_START_PERIOD;

bool antenna_did_deploy(void) {
	// if we fail to get the mutex, continue on and mess with anything reading sensors
	// (we don't care about sensors much relative to antenna deployment)
	bool got_mutex = false;
	if (xSemaphoreTake(processor_adc_mutex, HARDWARE_MUTEX_WAIT_TIME_TICKS)) {
		got_mutex = true;
	} else {
		log_error(ELOC_ANTENNA_DEPLOY, ECODE_PROC_ADC_MUTEX_TIMEOUT, true);
	}

	_set_5v_enable_unsafe(true);
	bool did_deploy = get_input(P_DET_RTN);
	_set_5v_enable_unsafe(false);

	if (got_mutex) xSemaphoreGive(processor_adc_mutex);
	return did_deploy;
}

bool try_pwm_deploy(long pin, long pin_mux, int ms, uint8_t p_ant) {
	bool was_deployed = antenna_did_deploy();
	configure_pwm(pin, pin_mux, p_ant);

	bool got_hw_state_mutex = hardware_state_mutex_take(ELOC_ANTENNA_DEPLOY);
	enable_pwm(current_on_cycle);
	get_hw_states()->antenna_deploying = true;
	if (got_hw_state_mutex) hardware_state_mutex_give();

	vTaskDelay(ms / portTICK_PERIOD_MS);

	// read current (both just in case) so we can shut it down if we need
	uint16_t ad7991_bat_results[4];
	uint16_t li1, li2, lf1, lf2, lfo1, lfo2;	
	if (p_ant == 1) {
		// TODO: check mutex value?
		//read_lion_current_precise(&li1, &li2);
		read_ad7991_batbrd_precise(ad7991_bat_results);
		li1 = ad7991_bat_results[1];
		li2 = ad7991_bat_results[0];
	} else {
		read_lifepo_current_precise(&lf1, &lfo1, &lf2, &lfo2);
	}
	
	got_hw_state_mutex = hardware_state_mutex_take(ELOC_ANTENNA_DEPLOY);
	disable_pwm();
	get_hw_states()->antenna_deploying = false;
	if (got_hw_state_mutex) hardware_state_mutex_give();

	bool can_cont = true;
	bool deployed = antenna_did_deploy();
	// Check if we're past max current and need to move on to the next pin
	// Also check if we were below the minimum expected current and log an error if we were (low priority)
	if (p_ant == 1) {
		print("PWM was on LiON\nCurrent on 1: %d\nCurrent on 2: %d\n", li1, li2);
		uint16_t li_cur;
		li_discharging_t lid = get_li_discharging();
		if (lid == LI1_DISG) {
			li_cur = li1;
		} else if (lid == LI2_DISG) {
			li_cur = li2;
		} else {
			li_cur = li1 + li2;
		}

		if (li_cur < PWM_MAX_CUR_L || (deployed && !was_deployed)) {
			can_cont = false;
		} else if (deployed && li_cur > PWM_VERY_MIN_CUR_L) {
			log_error(ELOC_ANTENNA_DEPLOY, ECODE_PWM_CUR_VERY_LOW_ON_DEPLOY, false);
		} else if (deployed && li_cur > PWM_MIN_CUR_L) {
			log_error(ELOC_ANTENNA_DEPLOY, ECODE_PWM_CUR_LOW_ON_DEPLOY, false);
		}
	} else {
		print("PWM was on LiFePO4\nCurrent on bank 1: %d\n", lf1);
		if (lf1 < PWM_MAX_CUR_LF || (deployed && !was_deployed)) {
			can_cont = false;
		} else if (deployed && lf1 < PWM_VERY_MIN_CUR_LF) {
			log_error(ELOC_ANTENNA_DEPLOY, ECODE_PWM_CUR_VERY_LOW_ON_DEPLOY, false);
		} else if (deployed && lf1 < PWM_MIN_CUR_LF) {
			log_error(ELOC_ANTENNA_DEPLOY, ECODE_PWM_CUR_LOW_ON_DEPLOY, false);
		}
	}
	
	if (can_cont) { // increment for next call
		current_on_cycle++;
	}
	// it shouldn't be on too much, so if it's at 14 switch to the next pin
	if (current_on_cycle >= (PWM_PERIOD - 2) || !can_cont) {
		can_cont = false;
		uint8_t ecode;
		switch (curren_pwm_pin) {
			case 1:
				ecode = ECODE_P1_NOT_DEPLOYED;
				break;
			case 2:
				ecode = ECODE_P2_NOT_DEPLOYED;
				break;
			case 3:
				ecode = ECODE_P3_NOT_DEPLOYED;
				break;
		}
		log_error(ELOC_ANTENNA_DEPLOY, ecode, true);
		current_on_cycle = PWM_START_PERIOD;
		curren_pwm_pin++;
		// now if the pin is past 3, set it back to 1
		if (curren_pwm_pin > 3) {
			curren_pwm_pin = 1;
		}
	}
	return !can_cont;
}

int get_current_pwm_pin(void) {
	return curren_pwm_pin;
}
