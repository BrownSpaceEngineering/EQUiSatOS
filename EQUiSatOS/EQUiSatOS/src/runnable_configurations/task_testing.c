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
	// pvParameters is a pointer (4 BYTES)
	
	// initialize xNextWakeTime once
	// = 2 BYTES if configUSE_16_BIT_TICKS else 4 BYTES
	// right now, it's 4 BYTES
	TickType_t xNextWakeTime = xTaskGetTickCount();
	
	uint16_t val1 = 10;// 2 BYTES
	// total of 10 BYTES here
	// start to overflow
	uint16_t val2 = 11;
	uint16_t val3 = 12;
	uint16_t val4 = 13;
	// see with breakpoints whether RTOS crashes if you go over the stack size

	for( ;; )
	{
		// block for a time
		vTaskDelayUntil( &xNextWakeTime, 1000 / portTICK_PERIOD_MS);

	}
	// delete this task if it ever breaks out
	vTaskDelete( NULL );
}