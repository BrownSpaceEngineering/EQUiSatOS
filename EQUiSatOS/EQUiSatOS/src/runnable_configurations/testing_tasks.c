/*
 * testing_tasks.c
 *
 * Created: 1/30/2017 3:56:53 PM
 *  Author: mckenna
 */

#include "testing_tasks.h"

/* task constants */
#define TESTING_TASK_FREQ			1000
#define TESTING_TASK_STACK_SIZE		512

// task functions / handles
TaskHandle_t suicide_test_handle;
void testing_task(void *pvParameters);
void task_suicide_test(void *pvParameters);
void task_stack_size_overflow_test(void *pvParameters);

// task data
#ifdef RUN_TESTING_TASKS
	StackType_t testing_task_stack[TESTING_TASK_STACK_SIZE];
	StaticTask_t testing_task_buffer;
#endif

void create_testing_tasks(void) 
{
	#ifdef RUN_TESTING_TASKS
		xTaskCreateStatic(testing_task,
			"testing task",
			TESTING_TASK_STACK_SIZE,
			NULL,
			STATE_HANDLING_PRIORITY, //tskIDLE_PRIORITY, // may change with different tests
			testing_task_stack,
			&testing_task_buffer);
	
	// 	xTaskCreate(task_suicide_test,
	// 	 	"task suicide testing",
	// 	 	1024,
	// 	 	NULL,
	// 	 	DATA_READ_PRIORITY,
	// 		suicide_test_handle);
	// 	
	// 	xTaskCreate(task_stack_size_overflow_test,
	// 	 	"task stack size purpose test",
	// 	 	320/sizeof(portSTACK_TYPE), // 10 bytes of stack space
	// 	 	NULL,
	// 	 	tskIDLE_PRIORITY);
	#endif
}

void testing_task(void *pvParameters) 
{
	// initialize xNextWakeTime once
	TickType_t xNextWakeTime = xTaskGetTickCount();
	
	for ( ;; )
	{
		vTaskDelayUntil( &xNextWakeTime, TESTING_TASK_FREQ / portTICK_PERIOD_MS);
		
		// callback testing function in main
		run_rtos_tests();
		
		/************************************************************************/
		/* MISC                                                                 */
		/************************************************************************/
		//test_message_packaging();
		//stress_test_message_packaging();
		vTaskDelay(2000);
		//configASSERT(check_task_state_consistency());
		
		/************************************************************************/
		/* STATE CHANGES														*/
		/************************************************************************/
		// 	test_normal_satellite_state_sequence();
		//	test_all_state_transitions();
		// 	test_watchdog_reset_bat_charging();
		// 	test_watchdog_reset_attitude_data();
		// 	test_watchdog_reset_antenna_deploy();
		// 	test_watchdog_reset_transmit_task();
		// 	test_watchdog_reset_idle_data_task();
		// 	test_watchdog_reset_flash_activate_task();
		// 	test_watchdog_reset_low_power_data_task();
	}
	// delete this task if it ever breaks out
	vTaskDelete( NULL );
}

void task_suicide_test(void *pvParameters) 
{
	// initialize xNextWakeTime once
	TickType_t xNextWakeTime = xTaskGetTickCount();
	
	int i = 0;

	for( ;; )
	{
		// block for a time 
		vTaskDelayUntil( &xNextWakeTime, 1000 / portTICK_PERIOD_MS);
		
		while (i <= 100000) {
			if (i >= 50000) {
				vTaskSuspend(suicide_test_handle);
				//int b = 0;
			}	
			i++;
		}
		
		//int b = 0;
		
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