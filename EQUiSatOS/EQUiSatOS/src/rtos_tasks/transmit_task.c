/*
 * transmit_task.c
 *
 * Created: 9/21/2017 20:39:34
 *  Author: mcken
 */ 

#include "rtos_tasks.h"
#include "stacks/package_transmission.h"


void write_message_top(uint8_t num_data, size_t size_data) {
	// write preamble
	write_preamble(get_current_timestamp(), CurrentState, 
		num_data * size_data);
	
	// grab idle data and write to header
	idle_data_t* cur_data = equistack_Get(&idle_readings_equistack, 0);
	write_header(cur_data);
				
	// TOOD: grab errors and write to section
	//write_errors(&errors_equistack);
}

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
			
			// based on what state we're in, compile a different message
			switch(nextState)
			{
				case ATTITUDE_DATA: ; // empty statement to allow definition
				
					write_message_top(ATTITUDE_DATA_PACKETS, ATTITUDE_DATA_PACKET_SIZE);
					write_attitude_data(&attitude_readings_equistack);
					break;
					
					// TODO: like to have a get_msg_buffer_data() that returns the pointer to the data section of the buffer
					// attitude_data_t* attitude_data_trans = NULL; // (attitude_data_t*) get_msg_buffer_data();
				
					// assert(ATTITUDE_DATA_PACKETS <= ATTITUDE_STACK_MAX);
					// for (uint8_t i = 0; i < ATTITUDE_DATA_PACKETS; i++) {
					//	// TODO: What's with first arg?
					//	memcpy(&(attitude_data_trans[i]), equistack_Get(&attitude_readings_equistack, 0), sizeof(attitude_data_t));
					//}
				
					//if (attitude_data_trans != NULL) { validDataTransmitted = true; } // TODO: If we got any invalid data (all null?)... stop?
					
				case TRANSMIT_DATA: ; // empty statement to allow definition
					
					write_message_top(TRANSMIT_DATA_PACKETS, TRANSMIT_DATA_PACKET_SIZE);
					write_transmit_data(&transmit_readings_equistack);
					break;
					
				case FLASH_DATA: ; // empty statement to allow definition
				
					write_message_top(FLASH_DATA_PACKETS, FLASH_DATA_PACKET_SIZE);
					write_flash_data(&flash_readings_equistack);
					break;
					
				default:
					validDataTransmitted = true; // if the state equistack is empty, we have no data, so avoid looping until we get some (potentially infinitely)
			};
		} while (!validDataTransmitted);
		
		task_suspend(TRANSMIT_DATA_TASK);
	}
	// delete this task if it ever breaks out
	vTaskDelete( NULL );
}