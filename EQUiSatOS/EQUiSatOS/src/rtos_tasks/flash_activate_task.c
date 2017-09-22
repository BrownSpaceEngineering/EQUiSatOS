/*
 * flash_activate_task.c
 *
 * Created: 9/21/2017 20:40:34
 *  Author: mcken
 */ 

#include "rtos_tasks.h"

void flash_activate_task(void *pvParameters)
{
	// initialize xNextWakeTime onces
	TickType_t xNextWakeTime = xTaskGetTickCount();
	
	for ( ;; )
	{
		vTaskDelayUntil( &xNextWakeTime, FLASH_ACTIVATE_TASK_FREQ / portTICK_PERIOD_MS);
		
		taskResumeIfSuspended(flash_data_task_handle, FLASH_DATA_TASK);
		
		// TODO: actually flash leds
		
		vTaskSuspend(flash_data_task_handle);
	}
	// delete this task if it ever breaks out
	vTaskDelete( NULL );
}