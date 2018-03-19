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

static uint8_t num_tries = 0;

bool should_exit_antenna_deploy(void) {
	bool probably_deployed = (antenna_did_deploy() && num_tries > 0); // must try whole range of pin 1		
	if (probably_deployed) {
		//update the event history before ending antenna deploy task
		update_sat_event_history(1, 0, 0, 0, 0, 0, 0);
	}		
	return probably_deployed || num_tries >= 4;
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
		
		// report to watchdog (again)
		report_task_running(ANTENNA_DEPLOY_TASK);
		
		bool did_deploy = antenna_did_deploy();
		if (num_tries == 0 && did_deploy) {
			log_error(ELOC_ANTENNA_DEPLOY, ECODE_DET_ALREADY_HIGH, false);
		} else if (num_tries > 0 && did_deploy) {
			// then the antenna should actually be deployed
			// (delay until the state handling task suspends us)
			print("Deployed!\n");			
			update_sat_event_history(1, 0, 0, 0, 0, 0, 0);
			vTaskDelayUntil(&prev_wake_time, ANTENNA_DEPLOY_TASK_LESS_FREQ / portTICK_PERIOD_MS);
			// report to watchdog (again)
			report_task_running(ANTENNA_DEPLOY_TASK);
			continue;
		}
		
		int current_pwm_pin = get_current_pwm_pin();
		if (current_pwm_pin == 1) {
			uint16_t li1, li2;
			read_lion_volts_precise(&li1, &li2, true);
			li_discharging_t lid = get_li_discharging();
			if ((lid == LI1_DISG && li1 > PWM_LION_MIN_V)
					|| (lid == LI2_DISG && li2 > PWM_LION_MIN_V)
					|| (lid == BOTH_DISG && li1 + li2 > PWM_LION_MIN_V)) {
				if (xSemaphoreTake(critical_action_mutex, CRITICAL_MUTEX_WAIT_TIME_TICKS)) {
					if (try_pwm_deploy(P_ANT_DRV1, P_ANT_DRV1_MUX, PWM_LENGTH_MS, 1)) {
						num_tries++;
					}
					
					xSemaphoreGive(critical_action_mutex);
				} else {
					log_error(ELOC_ANTENNA_DEPLOY, ECODE_CRIT_ACTION_MUTEX_TIMEOUT, false);
				}
				
				// report to watchdog (again)
				report_task_running(ANTENNA_DEPLOY_TASK);
			} else {
				vTaskDelay(ANTENNA_DEPLOY_LI_NOT_CHARGED_WAIT / portTICK_PERIOD_MS);
			}
		} else {
			uint16_t lf1, lf2, lf3, lf4;
			read_lifepo_volts_precise(&lf1, &lf2, &lf3, &lf4, true);
			if (lf1 + lf2 > PWM_LIFEPO_MIN_V) {
				if (xSemaphoreTake(critical_action_mutex, CRITICAL_MUTEX_WAIT_TIME_TICKS)) {
					int pin = current_pwm_pin == 2 ? P_ANT_DRV2 : P_ANT_DRV3;
					int mux = current_pwm_pin == 2 ? P_ANT_DRV2_MUX : P_ANT_DRV3_MUX;
					set_output(true, P_LF_B1_OUTEN);
					bool is_chgn = get_output(P_LF_B1_RUNCHG);
					if (is_chgn) {
						set_output(false, P_LF_B1_CHGN);
					}
					vTaskDelay(10); // delay to let the bank turn on
					if (try_pwm_deploy(pin, mux, PWM_LENGTH_MS, current_pwm_pin)) {
						num_tries++;
					}
					set_output(false, P_LF_B1_OUTEN);
					set_output(is_chgn, P_LF_B1_CHGN);
					
					xSemaphoreGive(critical_action_mutex);
					
					// report to watchdog (again)
					report_task_running(ANTENNA_DEPLOY_TASK);
					
				} else {
					log_error(ELOC_ANTENNA_DEPLOY, ECODE_CRIT_ACTION_MUTEX_TIMEOUT, false);
				}
			} else {
				vTaskDelay(ANTENNA_DEPLOY_LF_NOT_CHARGED_WAIT / portTICK_PERIOD_MS);
			}
		}
	}
	
	// delete this task if it ever breaks out
	vTaskDelete(NULL);
}