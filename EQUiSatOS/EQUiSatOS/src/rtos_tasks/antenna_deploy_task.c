/*
 * antenna_deploy_task.c
 *
 * Created: 9/21/2017 20:36:14
 *  Author: mcken
 */ 

#include "rtos_tasks.h"
#include "../config/proc_pins.h"
#include "../processor_drivers/PWM_Commands.h"

static int num_tries = 0;
void antenna_deploy_task(void *pvParameters) {
	TickType_t xNextWakeTime = xTaskGetTickCount();

	for( ;; )
	{
		vTaskDelayUntil(&xNextWakeTime, ANTENNA_DEPLOY_TASK_FREQ / portTICK_PERIOD_MS);
		// set DET_RTN (antenna deployment pin) as input
		setup_pin(false, P_DET_RTN);
		// if it's open kill the task because the antenna has been deployed
		// or kill it if it's run more than 5 times because it's a lost cause
		if (get_input(P_DET_RTN) || num_tries >= 5) {
			vTaskDelete(NULL);
		} else {
			int mod_tries = num_tries % 3;
			if (mod_tries == 0 && true /* LiON is sufficiently charged and enough time has passed*/) {
				configure_pwm(P_ANT_DRV1, P_ANT_DRV1_MUX);
				int start = get_count();
				while (get_count() < start + 3) {
					set_pulse_width_fraction(3, 4);
				}
			} else if (mod_tries != 0 && true /*LiFePO is sufficiently charged and enough time has passed*/){
				if (mod_tries == 1) {
					configure_pwm(P_ANT_DRV2, P_ANT_DRV2_MUX);
					int start = get_count();
					while (get_count() < start + 3) {
						set_pulse_width_fraction(3, 4);
					}
				} else {
					configure_pwm(P_ANT_DRV3, P_ANT_DRV3_MUX);
					int start = get_count();
					while (get_count() < start + 3) {
						set_pulse_width_fraction(3, 4);
					}
				}
			}
			num_tries++;
		}
	}
	
	// delete this task if it ever breaks out
	vTaskDelete(NULL);
}