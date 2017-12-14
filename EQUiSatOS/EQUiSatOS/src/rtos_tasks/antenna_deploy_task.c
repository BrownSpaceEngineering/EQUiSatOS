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

void antenna_deploy_task(void *pvParameters) {
	TickType_t prev_wake_time = xTaskGetTickCount();
	
	init_task_state(ANTENNA_DEPLOY_TASK); // suspend or run on boot

	for( ;; )
	{		
		vTaskDelayUntil(&prev_wake_time, ANTENNA_DEPLOY_TASK_FREQ / portTICK_PERIOD_MS);
		
		// report to watchdog
		report_task_running(ANTENNA_DEPLOY_TASK);
		
		// if it's open kill the task because the antenna has been deployed
		// or kill it if it's run more than 5 times because it's a lost cause
		if ((get_input(P_DET_RTN) && num_tries > 0) || num_tries >= 5) {
			// switch states, suspending this task in the process
			set_sat_state(HELLO_WORLD);
		} else {
			if (true /* TODO: LiON is sufficiently charged*/) {
				try_pwm_deploy(P_ANT_DRV1, P_ANT_DRV1_MUX, DEFAULT_PWM_MS_RUN);
				try_pwm_deploy(P_ANT_DRV2, P_ANT_DRV2_MUX, DEFAULT_PWM_MS_RUN);
				try_pwm_deploy(P_ANT_DRV3, P_ANT_DRV3_MUX, DEFAULT_PWM_MS_RUN);
				num_tries++;
			}
		}
	}
	
	// delete this task if it ever breaks out
	vTaskDelete(NULL);
}