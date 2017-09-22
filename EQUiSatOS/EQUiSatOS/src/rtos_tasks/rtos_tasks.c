/*
 * rtos_tasks.c
 *
 * Created: 9/27/2016 8:21:58 PM
 *  Author: rj16
 */ 

#include "rtos_tasks.h"
#include "processor_drivers\USART_Commands.h"

/************************************************************************/
/* Helper Functions														*/
/************************************************************************/
void taskResumeIfSuspended(TaskHandle_t task_handle, task_type_t taskId) 
{
	if (task_handle != NULL && eTaskGetState(task_handle) == eSuspended) 
	{
		vTaskResume(task_handle); // actually resume task
		TaskSuspendedStates |= (1 << taskId); // note we WERE suspended
	}
}

/* Checks and returns whether this task was suspended, AND report that it is not suspended */
bool checkIfSuspendedAndUpdate(task_type_t taskId) {
	bool val = TaskSuspendedStates & (1 << taskId); // check the state (>0 if was suspended)
	TaskSuspendedStates &= ~(1 << taskId); // set our suspended bit to 0
	return val;
}

void increment_data_type(uint16_t data_type, int *data_array_tails, int *loops_since_last_log)
{
	// increment array tail marker and reset reads-per-log counter
	data_array_tails[data_type] = data_array_tails[data_type] + 1;
	loops_since_last_log[data_type] = 0;
}

uint32_t get_current_timestamp()
{
	// TODO: get a more accurate and persistent timestamp (relative to an alive message)
	return xTaskGetTickCount(); // represents the ms from vstartscheduler
}

void increment_all(uint8_t* int_arr, uint8_t length)
{
	for(uint8_t i = 0; i < length; i++)
	{
		int_arr[i] = int_arr[i] + 1;
	}
}

void set_all(uint8_t* int_arr, uint8_t length, int value)
{
	for(uint8_t i = 0; i < length; i++)
	{
		int_arr[i] = value;
	}
}