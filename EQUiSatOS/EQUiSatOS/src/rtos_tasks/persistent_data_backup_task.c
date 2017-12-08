/*
 * persistent_data_backup_task.c
 *
 * Created: 12/8/2017 01:40:22
 *  Author: mcken
 */ 

#include "rtos_tasks.h"

// super simple task that periodically writes satellite state to non-volatile memory
void persistent_data_backup_task(void *pvParameters) {
	TickType_t prev_wake_time = xTaskGetTickCount();

	for( ;; )
	{
		vTaskDelayUntil( &prev_wake_time, PERSISTENT_DATA_BACKUP_TASK_FREQ / portTICK_PERIOD_MS);
		write_state_to_storage();
	}
	
	// delete this task if it ever breaks out
	vTaskDelete( NULL );
}