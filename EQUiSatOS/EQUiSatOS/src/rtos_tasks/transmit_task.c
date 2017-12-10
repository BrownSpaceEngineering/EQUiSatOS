/*
 * transmit_task.c
 *
 * Created: 9/21/2017 20:39:34
 *  Author: mcken
 */

#include "rtos_tasks.h"
#include "data_handling/package_transmission.h"

/**
 * Variables
 */
// define buffer here to keep LOCAL
uint8_t msg_buffer_1[MSG_BUFFER_SIZE];
uint8_t msg_buffer_2[MSG_BUFFER_SIZE];
uint8_t msg_buffer_3[MSG_BUFFER_SIZE];

// references to what message type each of the buffers is designated for right now
msg_data_type_t buffer_1_msg_type = IDLE_DATA;		// 0
msg_data_type_t buffer_2_msg_type = ATTITUDE_DATA;	// 1
msg_data_type_t buffer_3_msg_type = FLASH_DATA;		// 2

// count of how many times the current packet has been transmitted
uint8_t current_sequence_transmissions = 0;

/** 
 * Attempts to find a message type after prev_msg_type 
 * (wrapping around the end of the list, and possibly equal to prev_msg_type),
 * that has a non-empty equistack.
 * Returns that message type.
 * NOTE: this function could lock all the equistack mutexes to be certain of sizes,
 * but it's not necessary
 */
msg_data_type_t determine_single_msg_to_transmit(msg_data_type_t prev_msg_type) {
	configASSERT(prev_msg_type != (msg_data_type_t) -1);
	
	// special case for LOW_POWER mode;
	// only transmit LOW_POWER packets unless the equistack is empty
	if (get_sat_state() == LOW_POWER) {
		equistack* low_power_equistack = get_msg_type_equistack(LOW_POWER_DATA);
		configASSERT(low_power_equistack != NULL);
		if (low_power_equistack->cur_size > 0) {
			return LOW_POWER_DATA;
		}
		return -1;
	}
	
	msg_data_type_t new_msg_type;
	do {
		// attempt to get next message type, wrapping around
		new_msg_type = (prev_msg_type + 1) % NUM_MSG_TYPE;
		
		// skip LOW_POWER_DATA
		if (new_msg_type == LOW_POWER_DATA) {
			continue;
		}
		
		equistack* msg_type_equistack = get_msg_type_equistack(new_msg_type);
		configASSERT(msg_type_equistack != NULL);
		if (msg_type_equistack->cur_size > 0) {
			return new_msg_type;
		}
	} while (new_msg_type != prev_msg_type);
	
	// if we didn't find any and ran back into the prev_msg_type, all equistacks are empty
	return -1;
}

/**
 * Determines what message types (three slots) to transmit, based on what data we have.
 * Returns whether anything can be transmitted.
 */
bool determine_data_to_transmit() {
	// only grab new transmission and fill message buffer after we've transmitted this one enough
	if (current_sequence_transmissions >= TRANSMIT_TASK_MSG_REPEATS) {
		current_sequence_transmissions = 0;

		// determine which message type we should transmit,
		// attempting to transmit an increasing sequence of message type
		// (if some or all of the equistacks are empty, then this sequence may not
		//  be perfectly sequential, i.e. there may be duplicates or skipped values)
		msg_data_type_t new_buffer_1_msg_type, new_buffer_2_msg_type, new_buffer_3_msg_type;
		new_buffer_1_msg_type = determine_single_msg_to_transmit(buffer_1_msg_type);
		
		if (new_buffer_1_msg_type == (msg_data_type_t) -1) {
			// if all equistacks are empty (no message type had a full one), don't transmit
			return false;
		} else {
			// otherwise, continue trying to transmit, optimistically transmitting the packet
			// type right after the previous one, or in the worst case transmitting 
			// all of the same packet
			new_buffer_2_msg_type = determine_single_msg_to_transmit(new_buffer_1_msg_type);
			new_buffer_3_msg_type = determine_single_msg_to_transmit(new_buffer_2_msg_type);
			configASSERT(new_buffer_3_msg_type != (msg_data_type_t) -1);
			
			buffer_1_msg_type = new_buffer_1_msg_type;
			buffer_2_msg_type = new_buffer_2_msg_type;
			buffer_3_msg_type = new_buffer_3_msg_type;
			
 			return true;
		}
	}
	return true;
}

void transmit_task(void *pvParameters)
{
	// initialize xNextWakeTime once
	TickType_t prev_wake_time = xTaskGetTickCount();

	init_task_state(TRANSMIT_TASK); // suspend or run on boot

	for( ;; )
	{
		// block for a time based on this task's globally-set frequency
		// (Note: changes to the frequency can be delayed in taking effect by as much as the past frequency...)
		vTaskDelayUntil( &prev_wake_time, TRANSMIT_TASK_FREQ / portTICK_PERIOD_MS);

		// report to watchdog
		report_task_running(TRANSMIT_TASK);
		
		if (!determine_data_to_transmit()) {
			// don't transmit right now
			continue;
		}
		
		// actually write buffers
		uint32_t current_timestamp = get_current_timestamp();
		write_packet(msg_buffer_1, buffer_1_msg_type, current_timestamp);
		write_packet(msg_buffer_2, buffer_2_msg_type, current_timestamp);
		write_packet(msg_buffer_3, buffer_3_msg_type, current_timestamp);

		// actually send buffer over USART to radio for transmission
		// TODO: We will want to wait here between calls to allow for actual data transfer
		// (this can be calculated from the baud rate)
		usart_send_string(msg_buffer_1);
		usart_send_string(msg_buffer_2);
		usart_send_string(msg_buffer_3);

		current_sequence_transmissions++; // NOTE: putting this here as opposed to after the confirmation
										// means we will try to send the newest packet if encountering failures,
										// as opposed to the first one to fail.

		bool transmission_in_progress = true;
		TickType_t start_tick = xTaskGetTickCount();
		while (transmission_in_progress) {
			// give control back to RTOS to let transmit data task read info
			vTaskDelayUntil( &prev_wake_time, TRANSMIT_TASK_TRANS_MONITOR_FREQ / portTICK_PERIOD_MS);

			transmission_in_progress = false; // TODO: query radio state / current

			// if MS equivalent of # of ticks since start exceeds timeout, quit and note error
			if ((xTaskGetTickCount() - start_tick) * portTICK_PERIOD_MS > TRANSMIT_TASK_CONFIRM_TIMEOUT) {
				log_error(ELOC_RADIO, ECODE_TRANS_CONFIRM_TIMEOUT, TRUE); // TODO: not the best error location
				break;
			}
		}
	}
	// delete this task if it ever breaks out
	vTaskDelete( NULL );
}
