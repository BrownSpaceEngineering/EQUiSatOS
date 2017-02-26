/*
 * task_testing.c
 *
 * Created: 1/30/2017 3:56:53 PM
 *  Author: mckenna
 */

#include "task_testing.h"

int i = 0;

void task_suicide_test(void *pvParameters) 
{
	// initialize xNextWakeTime once
	TickType_t xNextWakeTime = xTaskGetTickCount();

	for( ;; )
	{
		// block for a time 
		vTaskDelayUntil( &xNextWakeTime, 1000 / portTICK_PERIOD_MS);
		
		while (i <= 100000) {
			if (i >= 50000) {
				vTaskSuspend(suicide_test_handle);
				int b = 0;
			}	
			i++;
		}
		
		int b = 0;
		
	}
	// delete this task if it ever breaks out
	vTaskDelete( NULL );	
}


void task_stack_size_overflow_test(void *pvParameters)
{
	// initialize xNextWakeTime once
	TickType_t xNextWakeTime = xTaskGetTickCount();

	for( ;; )
	{
		// block for a time
		vTaskDelayUntil( &xNextWakeTime, 1000 / portTICK_PERIOD_MS);
		
		
		
	}
	// delete this task if it ever breaks out
	vTaskDelete( NULL );
}