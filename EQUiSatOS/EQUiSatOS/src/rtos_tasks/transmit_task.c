/*
 * transmit_task.c
 *
 * Created: 9/21/2017 20:39:34
 *  Author: mcken
 */ 

#include "rtos_tasks.h"
#include "stacks/package_transmission.h"

// define buffer here to keep it LOCAL
char *msg_buffer [MSG_BUFFER_SIZE];

void write_message_top(uint8_t num_data, size_t size_data) {
	// write preamble
	write_preamble(msg_buffer, get_current_timestamp(), CurrentState, 
		num_data * size_data);
	
	// grab idle data and write to header
	idle_data_t* cur_data = equistack_Get(&idle_readings_equistack, 0);
	write_header(msg_buffer, cur_data);
				
	// TOOD: grab errors and write to section
	//write_errors(buffer, &errors_equistack);
}

void transmit_task(void *pvParameters)
{
	// initialize xNextWakeTime once
	TickType_t xNextWakeTime = xTaskGetTickCount();
	
	// count of how many times the current packet has been transmitted
	uint8_t current_packet_transmissions = 0;
	
	// this task is not initially active, so suspend itself initially
	//vTaskSuspend(NULL); 
	
	for( ;; )
	{
		// block for a time based on this task's globally-set frequency
		// (Note: changes to the frequency can be delayed in taking effect by as much as the past frequency...)
		vTaskDelayUntil( &xNextWakeTime, TRANSMIT_TASK_FREQ / portTICK_PERIOD_MS);
		
		// start up the data collection task so that its records data while we're transmitting
		task_resume_if_suspended(TRANSMIT_DATA_TASK);
		
		// only grab new transmission and fill message buffer after we've transmitted this one enough
		if (current_packet_transmissions >= TRANSMIT_TASK_MSG_REPEATS) {
			current_packet_transmissions = 0;
			
			// read the next state to transmit (get first off state queue)
			int nextState = (int*) equistack_Get(&last_reading_type_equistack, 0);
			
			// if there's no data in the state stack, we have nothing to transmit
			if (nextState != NULL) {
				// based on what state we're in, compile a different message
				switch(nextState)
				{
					case IDLE_DATA:
						write_message_top(IDLE_DATA_PACKETS, IDLE_DATA_PACKET_SIZE);
						write_attitude_data(msg_buffer, &idle_readings_equistack);
						break;
					
					case ATTITUDE_DATA:
						write_message_top(ATTITUDE_DATA_PACKETS, ATTITUDE_DATA_PACKET_SIZE);
						write_attitude_data(msg_buffer, &attitude_readings_equistack);
						break;
						
						//if (attitude_data_trans != NULL) { validDataTransmitted = true; } // TODO: If we got any invalid data (all null?)... stop?
					
					case FLASH_DATA:
						write_message_top(FLASH_DATA_PACKETS, FLASH_DATA_PACKET_SIZE);
						write_flash_data(msg_buffer, &flash_readings_equistack);
						break;
						
					case FLASH_CMP:
						write_message_top(FLASH_CMP_PACKETS, FLASH_CMP_PACKET_SIZE);
						write_flash_data(msg_buffer, &flash_cmp_readings_equistack);
						break;
					
					default:
						break;
				};
			}			
		}
		
		// actually send buffer over USART to radio for transmission
		usart_send_string(msg_buffer);
		current_packet_transmissions++; // NOTE: putting this here as opposed to after the confirmation
										// means we will try to send the newest packet if encountering failures,
										// as opposed to the first one to fail.
		
		bool transmission_in_progress = true;
		TickType_t start_tick = xTaskGetTickCount();
		while (transmission_in_progress) {
			// give control back to RTOS to let transmit data task read info
			vTaskDelayUntil( &xNextWakeTime, TRANSMIT_TASK_TRANS_MONITOR_FREQ / portTICK_PERIOD_MS);
			
			transmission_in_progress = false; // TODO: query radio state
			
			// if MS equivalent of # of ticks since start exceeds timeout, quit and note error
			if ((xTaskGetTickCount() - start_tick) * portTICK_PERIOD_MS > TRANSMIT_TASK_CONFIRM_TIMEOUT) {
				log_error(ELOC_PACKAGE_TRANS, ECODE_TRANS_CONFIRM_TIMEOUT); // TODO: not the best error location
				break;
			}
		}
		
		task_suspend(TRANSMIT_DATA_TASK);
	}
	// delete this task if it ever breaks out
	vTaskDelete( NULL );
}