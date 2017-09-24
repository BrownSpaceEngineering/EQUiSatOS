/*
 * transmit_task.c
 *
 * Created: 9/21/2017 20:39:34
 *  Author: mcken
 */ 

#include "rtos_tasks.h"
#include "stacks/package_transmission.h"

void transmit_task(void *pvParameters)
{
	// initialize xNextWakeTime once
	TickType_t xNextWakeTime = xTaskGetTickCount();

	for( ;; )
	{
		// block for a time based on this task's globally-set frequency
		// (Note: changes to the frequency can be delayed in taking effect by as much as the past frequency...)
		vTaskDelayUntil( &xNextWakeTime, TRANSMIT_TASK_FREQ / portTICK_PERIOD_MS);
		
		// start up the data collection task so that its records data while we're transmitting
		task_resume_if_suspended(TRANSMIT_DATA_TASK);

		// check that global buffer has not been broken (TODO: ERROR)
		assert(get_msg_buffer()[0] == 'K' && get_msg_buffer()[1] == '1' && get_msg_buffer()[2] == 'A' && get_msg_buffer()[3] == 'D');
		
		// TODO: Do we need this loop? Can we just assume we'll always have data??
		bool validDataTransmitted = false; // we're cynical
		do
		{
			// read the next state to transmit (get first off state queue)
			int nextState = (int*) equistack_Get(&last_reading_type_equistack, 0);
			
			// grab idle data and write to header
			idle_data_t* cur_data = equistack_Get(&idle_readings_equistack, 0);
			assert(MSG_HEADER_LENGTH == sizeof(idle_data_t));
			// TODO: like to have a get_msg_buffer_header() that returns the pointer to the header section of the buffer
			//memcpy(get_msg_buffer_header(), cur_data, HEADER_LENGTH);
			
			// TOOD: Get errors and add them (would also like to get an address of the error section)
			
			// based on what state we're in, compile a different message
			switch(nextState)
			{
				case ATTITUDE_DATA: ; // empty statement to allow definition
				// TODO: like to have a get_msg_buffer_data() that returns the pointer to the data section of the buffer
				attitude_data_t* attitude_data_trans = NULL; // (attitude_data_t*) get_msg_buffer_data();
				
				assert(ATTITUDE_DATA_PACKETS <= ATTITUDE_STACK_MAX);
				for (uint8_t i = 0; i < ATTITUDE_DATA_PACKETS; i++) {
					// TODO: What's with first arg?
					memcpy(&(attitude_data_trans[i]), equistack_Get(&attitude_readings_equistack, 0), sizeof(attitude_data_t));
				}
				
				//if (attitude_data_trans != NULL) { validDataTransmitted = true; } // TODO: If we got any invalid data (all null?)... stop?
				break;
				case TRANSMIT_DATA: ; // empty statement to allow definition
				break;
				case FLASH_DATA: ; // empty statement to allow definition
				break;
				default:
				validDataTransmitted = true; // if the state equistack is empty, we have no data, so avoid looping until we get some (potentially infinitely)
			};
		} while (!validDataTransmitted);
		
		vTaskSuspend(transmit_data_task_handle);
		
	}
	// delete this task if it ever breaks out
	vTaskDelete( NULL );
}