/*
 * battery_charging_task.c
 *
 * Created: 9/21/2017 20:37:56
 *  Author: mcken
 */ 

#include "rtos_tasks.h"

void battery_charging_task(void *pvParameters)
{
	// initialize xNextWakeTime onces
	TickType_t xNextWakeTime = xTaskGetTickCount();
	
	for ( ;; )
	{
		vTaskDelayUntil( &xNextWakeTime, BATTERY_CHARGING_TASK_FREQ / portTICK_PERIOD_MS);
		
		// TODO
	}
	// delete this task if it ever breaks out
	vTaskDelete( NULL );
}