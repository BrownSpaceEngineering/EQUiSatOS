/*
 * flash_activate_task.c
 *
 * Created: 9/21/2017 20:40:34
 *  Author: mcken
 */ 

#include "../processor_drivers/Flash_Commands.h"
#include "rtos_tasks.h"

#define NUM_FLASHES 3
#define TIME_BTWN_FLASHES 1000 // ms

void flash_activate_task(void *pvParameters)
{
	// initialize xNextWakeTime onces
	TickType_t xNextWakeTime = xTaskGetTickCount();
		
	init_task_state(FLASH_ACTIVATE_TASK); // suspend or run on boot
	
	for ( ;; )
	{	
		vTaskDelayUntil( &xNextWakeTime, FLASH_ACTIVATE_TASK_FREQ / portTICK_PERIOD_MS);
		
		// report to watchdog
		report_task_running(FLASH_ACTIVATE_TASK);
		
		// start taking data
		task_resume(FLASH_DATA_TASK);
		
		// actually flash leds
		for (int i = 0; i < NUM_FLASHES; i++) {
			flash_leds(); // note contains a delay of approx. 1 ms
			vTaskDelay(TIME_BTWN_FLASHES / portTICK_PERIOD_MS); // delay on last iteration as well
		}
		
		// stop taking data
		task_suspend(FLASH_DATA_TASK, false);
	}
	// delete this task if it ever breaks out
	vTaskDelete( NULL );
}