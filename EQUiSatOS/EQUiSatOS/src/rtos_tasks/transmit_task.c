/*
 * transmit_task.c
 *
 * Created: 9/21/2017 20:39:34
 *  Author: mcken
 */

#include "transmit_task.h"

/************************************************************************/
/* RADIO CONTROL FUNCTIONS                                              */
/************************************************************************/
enum radio_state {
	COMMAND_MODE,
	RECIEVING_COMMAND, // subset of COMMAND_MODE // TODO
	TX_RX_MODE,
	POWERED_OFF
} cur_radio_mode = POWERED_OFF;
#define POWER_ON_RADIO_MODE		TX_RX_MODE // TODO

void radio_control_init(void) {
	radio_init();
	radio_command_queue = xQueueCreateStatic(RADIO_CMD_QUEUE_LEN,
											 sizeof(radio_command_t),
											 _radio_command_queue_storage,
											 &_radio_command_queue_d);
}

// function to push radio command on to queue to be processed when ready
bool submit_radio_command(enum radio_command_type type, 
	void *input_arg, void *rx_data_dest, bool *rx_data_ready, TickType_t full_wait_time_ms) {
	
	struct radio_command_t cmd = {
		type, 
		input_arg,
		rx_data_dest,
		rx_data_ready
	};
	
	// note: cmd is copied so it's fine to use a local var
	return xQueueSendToFront(radio_command_queue, &cmd, full_wait_time_ms / portTICK_PERIOD_MS) == pdTRUE;
}

// performs the given radio command; returns whether the radio can accept more commands
bool process_radio_command(radio_command_t *command) {
	bool expecting_rx_data = false;
	switch (command->type) {
		// special commands
		case POWER_OFF:
			setRadioState(false, (bool) command->input_arg);
			cur_radio_mode = POWERED_OFF;
			return false;
		// POWER_ON handled separately
		
		// COMMAND_MODE commands
		case GET_TEMPERATURE:
			expecting_rx_data = true;
			XDL_get_temperature(); // XDL_get_temperature(command->rx_data_dest, command->rx_data_ready) TODO
			break;
			
		case COLD_RESET:
			expecting_rx_data = true;
			cold_reset();
			break;
			
		case WARM_RESET:
			expecting_rx_data = true;
			warm_reset();
			break;
			
		default:
			return true; // do nothing on bad command
	}

	// if there is data to be received, busy(ish) loop until then
	// TODO: timeout!!
	if (expecting_rx_data) {
		while (!receiveDataReady) {
			vTaskDelay(STATE_CHANGE_MONITOR_DELAY_TICKS);
		}
		if (command->rx_data_ready != NULL) 
			*(command->rx_data_ready) = true;
	}
	return true;
}

// function to process radio commands for a specified time period before
// returning radio to transmit state
void process_radio_commands(TickType_t prev_transmit_time) {
	static radio_command_t cur_command;
	
	// continue to try and process commands until we're close enough to the 
	// time we need to transmit that we need to get prepared
	// (if nothing is on / is added to the queue, we'll simply sleep during that time)
	TickType_t processing_deadline = prev_transmit_time +
		(TRANSMIT_TASK_FREQ - MAX_CMD_MODE_RECOVERY_TIME_MS) / portTICK_PERIOD_MS;

	while (xTaskGetTickCount() < processing_deadline) {
		// try to receive command from queue, waiting the maximum time we can before the processing deadline
		if (xQueueReceive(radio_command_queue, &cur_command, processing_deadline - xTaskGetTickCount())) {
			// turn on radio if it's not currently on
			setRadioState(true, true);
			// will delay while confirming
			cur_radio_mode = POWER_ON_RADIO_MODE;
			
			// activate command mode on the radio if we're not currently in it
			if (/* requires_command_mode(cur_command) && */ cur_radio_mode != COMMAND_MODE && cur_radio_mode != RECIEVING_COMMAND) {
				vTaskDelay(SET_CMD_MODE_WAIT_BEFORE_MS);
				set_command_mode(false); // don't delay, we'll take care of it
				vTaskDelay(SET_CMD_MODE_WAIT_AFTER_MS);
			}
			
			if (!process_radio_command(&cur_command)) {
				// power off command issued TODO: how to respond? probably suspend this task?
				// TODO
			}
		}
		// keep trying if nothing received
	}
	
	// warm reset the radio if necessary before moving to transmit state
	// so it can transmit
	if (cur_radio_mode != TX_RX_MODE) {
		warm_reset();
		vTaskDelay(WARM_RESET_WAIT_AFTER_MS / portTICK_PERIOD_MS);
	}
}

/************************************************************************/
/* DATA TRANSMISSION FUNCTIONS                                          */
/************************************************************************/
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

// attempts to send transmission
void attempt_transmission(void) {
	
	if (!determine_data_to_transmit()) {
		// don't transmit right now
		return;
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
	// wait here between calls to give buffer
	transmit_buf_wait(msg_buffer_1, MSG_SIZE);
	vTaskDelay(TIME_BTWN_MSGS_MS / portTICK_PERIOD_MS);
	transmit_buf_wait(msg_buffer_2, MSG_SIZE);
	vTaskDelay(TIME_BTWN_MSGS_MS / portTICK_PERIOD_MS);
	transmit_buf_wait(msg_buffer_3, MSG_SIZE);
	vTaskDelay(TIME_BTWN_MSGS_MS / portTICK_PERIOD_MS);

	// NOTE: putting this here as opposed to after the confirmation
	// means we will try to send the newest packet if encountering failures,
	// as opposed to the first one to fail.
	// we wrap around here so that the transmit buffers are updated each time
	// current_sequence_transmissions hits zero (mainly done so they're updated on first boot)
	current_sequence_transmissions = (current_sequence_transmissions + 1) % TRANSMIT_TASK_MSG_REPEATS;

	// TODO:::
//	bool transmission_in_progress = true;
// 	TickType_t start_tick = xTaskGetTickCount();
// 	while (transmission_in_progress) {
// 		// give control back to RTOS to let transmit data task read info
// 		vTaskDelayUntil( prev_wake_time, TRANSMIT_TASK_TRANS_MONITOR_FREQ / portTICK_PERIOD_MS);
// 
// 		transmission_in_progress = false; // TODO: query radio state / current
// 
// 		// if MS equivalent of # of ticks since start exceeds timeout, quit and note error
// 		if ((xTaskGetTickCount() - start_tick) * portTICK_PERIOD_MS > TRANSMIT_TASK_CONFIRM_TIMEOUT) {
// 			log_error(ELOC_RADIO, ECODE_CONFIRM_TIMEOUT, TRUE); // TODO: not the best error location
// 			return;
// 		}
// 	}
}

/************************************************************************/
/* MAIN RTOS TRANSMIT + RADIO GATEKEEPER TASK                           */
/************************************************************************/

void transmit_task(void *pvParameters)
{
	// delay to offset task relative to others, then start
	vTaskDelay(TRANSMIT_TASK_FREQ_OFFSET);
	TickType_t prev_transmit_time = xTaskGetTickCount();

	init_task_state(TRANSMIT_TASK); // suspend or run on boot

	// block initially so we don't go right into command-processing mode
	vTaskDelayUntil( &prev_transmit_time, TRANSMIT_TASK_FREQ / portTICK_PERIOD_MS); 

	for( ;; )
	{
		// report to watchdog
		report_task_running(TRANSMIT_TASK);
		
		/************************************************************************/
		/* STAGE ILLUSTRATION:                                                  */
		/* t			  t+x_1				   t+x_2		t+TRANSMIT_TASK_FREQ*/
		/* |-transmission-|--command handling--|--sleep---------|-transmission-|*/
		/* or												(cut short) 		*/
		/* |-transmission-|--command handling-------------------|-transmission-|*/
		/* (the difference is that not all the commands got handled in #2)		*/
		/************************************************************************/

		// enter command handling stage;
		// process any commands on the command queue for a maximum time period of 
		// approximately TRANSMIT_TASK_FREQ (minus expectations of transmission time)
		process_radio_commands(prev_transmit_time);
		
		// block for any leftover time
		vTaskDelayUntil( &prev_transmit_time, TRANSMIT_TASK_FREQ / portTICK_PERIOD_MS);
		
		// report to watchdog (again)
		report_task_running(TRANSMIT_TASK);
		
		// enter transmission stage;
		// after processing any radio commands, try a transmission
		attempt_transmission();
	}
	// delete this task if it ever breaks out
	vTaskDelete( NULL );
}

/************************************************************************/
/* UTILITY                                                              */
/************************************************************************/

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
	char* msg = "\nSent messages: %s %s %s\n";
	char* type1 = get_msg_type_str(buffer_1_msg_type);
	char* type2 = get_msg_type_str(buffer_2_msg_type);
	char* type3 = get_msg_type_str(buffer_3_msg_type);
	
	trace_print(msg, type1, type2, type3);
	print(msg, type1, type2, type3);
}
