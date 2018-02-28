/*
 * antenna_deploy_task.c
 *
 * Created: 9/21/2017 20:36:14
 *  Author: mcken
 */ 

#include "rtos_tasks.h"
#include "../config/proc_pins.h"
#include "../processor_drivers/PWM_Commands.h"
#include "../runnable_configurations/antenna_pwm.h"

static int num_tries = 0;

int num_tries_ant_deploy(void) {
	return num_tries;
}

void antenna_deploy_task(void *pvParameters) {
	// delay to offset task relative to others, then start
	vTaskDelay(ANTENNA_DEPLOY_TASK_FREQ_OFFSET);
	TickType_t prev_wake_time = xTaskGetTickCount();
	
	init_task_state(ANTENNA_DEPLOY_TASK); // suspend or run on boot

	for( ;; )
	{		
		// report to watchdog
		report_task_running(ANTENNA_DEPLOY_TASK);
		
		// only try quickly in ANTENNA_DEPLOY state, otherwise try more periodically
		if (get_sat_state() == ANTENNA_DEPLOY) {
			vTaskDelayUntil(&prev_wake_time, ANTENNA_DEPLOY_TASK_FREQ / portTICK_PERIOD_MS);
		} else {
			vTaskDelayUntil(&prev_wake_time, ANTENNA_DEPLOY_TASK_LESS_FREQ / portTICK_PERIOD_MS);
		}
		
		if (num_tries == 0 && get_input(P_DET_RTN)) {
			log_error(ELOC_ANTENNA_DEPLOY, ECODE_DET_ALREADY_HIGH, false);
		}
		
		int current_pwm_pin = get_current_pwm_pin();
		if (current_pwm_pin == 1) {
			uint16_t li1, li2;
			read_lion_volts_precise(&li1, &li2);
			if (li1 > PWM_LION_MIN_V || li2 > PWM_LION_MIN_V
					/* TODO: doesn't take into account which battery is discharging */) {
				if (xSemaphoreTake(critical_action_mutex, CRITICAL_MUTEX_WAIT_TIME_TICKS)) {
					try_pwm_deploy(P_ANT_DRV1, P_ANT_DRV1_MUX, PWM_LENGTH_MS, 1);
					
					xSemaphoreGive(critical_action_mutex);
				} else {
					log_error(ELOC_ANTENNA_DEPLOY, ECODE_CRIT_ACTION_MUTEX_TIMEOUT, true);
				}
				num_tries++;
			} else {
				vTaskDelay(1800000);
			}
		} else {
			uint16_t lf1, lf2, lf3, lf4;
			read_lf_volts_precise(&lf1, &lf2, &lf3, &lf4);
			if (lf1 + lf2 > PWM_LIFEPO_MIN_V) {
				if (xSemaphoreTake(critical_action_mutex, CRITICAL_MUTEX_WAIT_TIME_TICKS)) {
					int pin = current_pwm_pin == 2 ? P_ANT_DRV2 : P_ANT_DRV3;
					int mux = current_pwm_pin == 2 ? P_ANT_DRV2_MUX : P_ANT_DRV3_MUX;
					try_pwm_deploy(pin, mux, PWM_LENGTH_MS, current_pwm_pin);
					
					xSemaphoreGive(critical_action_mutex);
				} else {
					log_error(ELOC_ANTENNA_DEPLOY, ECODE_CRIT_ACTION_MUTEX_TIMEOUT, true);
				}
				num_tries++;
			} else {
				vTaskDelay(3600000);
			}
		}
	}
	
	// delete this task if it ever breaks out
	vTaskDelete(NULL);
}