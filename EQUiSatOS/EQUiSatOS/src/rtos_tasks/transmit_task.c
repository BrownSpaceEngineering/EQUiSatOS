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

int16_t non_transmitted_packets(equistack *msg_type_equistack, msg_data_type_t msg_type);

/** 
 * Attempts to find a message type after prev_msg_type 
 * (wrapping around the end of the list, and possibly equal to prev_msg_type),
 * that has a non-empty equistack.
 * If ignore_transmitted is set, doesn't count already-transmitted packets towards
 * an equistack's size.
 * Returns that message type.
 * NOTE: this function could lock all the equistack mutexes to be certain of sizes,
 * but it's not necessary
 */
msg_data_type_t determine_single_msg_to_transmit(msg_data_type_t prev_msg_type, bool ignore_transmitted) {
	configASSERT(prev_msg_type != (msg_data_type_t) -1);
	
	// special case for LOW_POWER mode;
	// only transmit LOW_POWER packets unless the equistack is empty
	// (it doesn't matter whether they're transmitted)
	if (get_sat_state() == LOW_POWER) {
		equistack* low_power_equistack = get_msg_type_equistack(LOW_POWER_DATA);
		configASSERT(low_power_equistack != NULL);
		if (low_power_equistack->cur_size > 0) {
			return LOW_POWER_DATA;
		}
		return -1;
	}
	
	msg_data_type_t new_msg_type = prev_msg_type;
	do {
		// attempt to get next message type, wrapping around
		new_msg_type = (new_msg_type + 1) % NUM_MSG_TYPE;
		
		// skip LOW_POWER_DATA
		if (new_msg_type == LOW_POWER_DATA) {
			continue;
		}

		equistack* msg_type_equistack = get_msg_type_equistack(new_msg_type);
		configASSERT(msg_type_equistack != NULL);
		if (ignore_transmitted) {
			// TODO: "non-transmitted" should really be considering packets that may be transmitted
			// elsewhere _in this sequence of 3 packets_ to avoid duplication when another type should be sent.
			if (non_transmitted_packets(msg_type_equistack, new_msg_type) > 0)
				return new_msg_type;
		} else {
			if (msg_type_equistack->cur_size > 0)
				return new_msg_type;
		}
	} while (new_msg_type != prev_msg_type);
	
	// if we didn't find any and ran back into the prev_msg_type, all equistacks are empty
	return -1;
}

// returns the number of non-transmitted packets in the given equistack
int16_t non_transmitted_packets(equistack *msg_type_equistack, msg_data_type_t msg_type) {
	int count = 0;
	for (int i = 0; i < msg_type_equistack->cur_size; i++) {
		// determine whether this packet is transmitted (dependent on type)
		bool transmitted;
		void* packet = equistack_Get(msg_type_equistack, i);
		switch (msg_type) {
			case IDLE_DATA:			transmitted = ((idle_data_t*)		packet)->transmitted;
			case ATTITUDE_DATA:		transmitted = ((attitude_data_t*)	packet)->transmitted;
			case FLASH_DATA:		transmitted = ((flash_data_t*)		packet)->transmitted;
			case FLASH_CMP_DATA:	transmitted = ((flash_cmp_data_t*)	packet)->transmitted;
			// may not ever happen
			case LOW_POWER_DATA:	transmitted = ((low_power_data_t*)	packet)->transmitted;
			case NUM_MSG_TYPE:
			default:
				transmitted = false;
		}
		
		if (!transmitted) count++;
	}
	return count;
}

/**
 * Determines what message types (three slots) to transmit, based on what data we have.
 * Returns whether anything should be transmitted.
 */
bool determine_data_to_transmit(void) {
	// only grab new transmission and fill message buffer after we've transmitted this one enough
	// (indicated by starting at or wrapping around to 0)
	if (current_sequence_transmissions == 0) {
		// determine which message type we should transmit,
		// attempting to transmit an increasing sequence of message type
		// (if some or all of the equistacks are empty, then this sequence may not
		//  be perfectly sequential, i.e. there may be duplicates or skipped values)
		msg_data_type_t new_buffer_1_msg_type, new_buffer_2_msg_type, new_buffer_3_msg_type;
		
		// initially, ignore transmitted packets when doing counts
		bool ignore_transmitted = true;
		new_buffer_1_msg_type = determine_single_msg_to_transmit(buffer_1_msg_type, ignore_transmitted); 

		if (new_buffer_1_msg_type == (msg_data_type_t) -1) {
			// if all equistacks have no messages that haven't been transmitted, 
			// try again but allow retransmission
			ignore_transmitted = false;
			new_buffer_1_msg_type = determine_single_msg_to_transmit(buffer_1_msg_type, ignore_transmitted);
			
			if (new_buffer_1_msg_type == (msg_data_type_t) -1) {
				// if all equistacks are COMPLETELY empty even considering already-transmitted (no message type had a full one), don't transmit
				return false;	
			}
		}
			
		// in all cases besides all stack empty, 
		// continue trying to transmit, optimistically transmitting the packet
		// type right after the previous one, or in the worst case transmitting 
		// all of the same packet
		new_buffer_2_msg_type = determine_single_msg_to_transmit(new_buffer_1_msg_type, ignore_transmitted);
		new_buffer_3_msg_type = determine_single_msg_to_transmit(new_buffer_2_msg_type, ignore_transmitted);
		configASSERT(new_buffer_3_msg_type != (msg_data_type_t) -1);
			
		buffer_1_msg_type = new_buffer_1_msg_type;
		buffer_2_msg_type = new_buffer_2_msg_type;
		buffer_3_msg_type = new_buffer_3_msg_type;
	}
	return true;
}

void debug_print_msg_types(void);

void transmit_task(void *pvParameters)
{
	// delay to offset task relative to others, then start
	vTaskDelay(TRANSMIT_TASK_FREQ_OFFSET);
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
		
		// double-make sure radio is set to transmit (don't check regulators every time, however)
		// (it should be on whenever this task is running anyways)
		setRadioState(true, false);
		
		// print a debug message
		debug_print_msg_types();
		
		// actually write buffers
		uint32_t current_timestamp = get_current_timestamp();
		write_packet(msg_buffer_1, buffer_1_msg_type, current_timestamp);
		write_packet(msg_buffer_2, buffer_2_msg_type, current_timestamp);
		write_packet(msg_buffer_3, buffer_3_msg_type, current_timestamp);
		
		// actually send buffer over USART to radio for transmission
		// TODO: We will want to wait here between calls to allow for actual data transfer
		// (this can be calculated from the baud rate)
		usart_send_buf(msg_buffer_1, MSG_SIZE);
		usart_send_buf(msg_buffer_2, MSG_SIZE);
		usart_send_buf(msg_buffer_3, MSG_SIZE);

		// NOTE: putting this here as opposed to after the confirmation
		// means we will try to send the newest packet if encountering failures,
		// as opposed to the first one to fail.
		// we wrap around here so that the transmit buffers are updated each time
		// current_sequence_transmissions hits zero (mainly done so they're updated on first boot)
		current_sequence_transmissions = (current_sequence_transmissions + 1) % TRANSMIT_TASK_MSG_REPEATS;

		bool transmission_in_progress = true;
		TickType_t start_tick = xTaskGetTickCount();
		while (transmission_in_progress) {
			// give control back to RTOS to let transmit data task read info
			vTaskDelayUntil( &prev_wake_time, TRANSMIT_TASK_TRANS_MONITOR_FREQ / portTICK_PERIOD_MS);

			transmission_in_progress = false; // TODO: query radio state / current

			// if MS equivalent of # of ticks since start exceeds timeout, quit and note error
			if ((xTaskGetTickCount() - start_tick) * portTICK_PERIOD_MS > TRANSMIT_TASK_CONFIRM_TIMEOUT) {
				log_error(ELOC_RADIO, ECODE_CONFIRM_TIMEOUT, TRUE); // TODO: not the best error location
				break;
			}
		}
	}
	// delete this task if it ever breaks out
	vTaskDelete( NULL );
}

char* get_msg_type_str(msg_data_type_t msg_type) {
	switch (msg_type) {
		case IDLE_DATA:			return "IDLE_DATA     ";
		case ATTITUDE_DATA:		return "ATTITUDE_DATA ";
		case FLASH_DATA:		return "FLASH_DATA    ";
		case FLASH_CMP_DATA:	return "FLASH_CMP_DATA";
		case LOW_POWER_DATA:	return "LOW_POWER_DATA";
		case NUM_MSG_TYPE:
		default:				return "INVALID TYPE  ";
	}
}

void debug_print_msg_types(void) {
	print("Sent messages: %s %s %s", 
		get_msg_type_str(buffer_1_msg_type),
		get_msg_type_str(buffer_2_msg_type),
		get_msg_type_str(buffer_3_msg_type));
}
