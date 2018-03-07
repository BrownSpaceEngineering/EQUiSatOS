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
// temporary version of radio temp; set periodically (right before radio transmit)
uint16_t radio_temp_cached;

uint16_t get_radio_temp_cached(void) {
	return radio_temp_cached;
}

// returns we've received a kill command and that command is currently active.
bool radio_is_killed(void) {
	// note: the cache is the most rad-tolerant area of the memory,
	// so we always grab the value from there
	return cache_get_radio_revive_timestamp() >= get_current_timestamp();
}

void read_radio_temp_mode(void) {
	// power on radio initially
	setRadioState(true, true);
	
	// set command mode to allow sending commands
	vTaskDelay(SET_CMD_MODE_WAIT_BEFORE_MS);
	set_command_mode(false); // don't delay, we'll take care of it
	vTaskDelay(SET_CMD_MODE_WAIT_AFTER_MS);

	clear_USART_rx_buffer();
	XDL_prepare_get_temp();
	usart_send_string(sendbuffer);
	vTaskDelay(TEMP_RESPONSE_TIME_MS / portTICK_PERIOD_MS);
	if (check_checksum(receivebuffer+1, 3, receivebuffer[4])) {
		radio_temp_cached = (receivebuffer[2] << 8) | receivebuffer[3];
	} else {
		//TODO: Wait longer?
		// log error
		log_error(ELOC_RADIO_TEMP, ECODE_TIMEOUT, false);
	}
	
	// warm reset to get back into transmit mode
	warm_reset();
	usart_send_string(sendbuffer);
	vTaskDelay(WARM_RESET_REBOOT_TIME / portTICK_PERIOD_MS);
	if (!check_checksum(receivebuffer+1, 1, receivebuffer[2]) && (receivebuffer[1] == 0)) {
		//power cycle radio
		setRadioPower(false);
		vTaskDelay(WARM_RESET_REBOOT_TIME / portTICK_PERIOD_MS);
		setRadioPower(true);
	}
	vTaskDelay(WARM_RESET_WAIT_AFTER_MS / portTICK_PERIOD_MS);
}

/************************************************************************/
/* UPLINK FUNCTIONS                                                     */
/************************************************************************/
// queue internals (put here because #includes)
StaticQueue_t _rx_command_queue_d;
uint8_t _rx_command_queue_storage[RX_CMD_QUEUE_LEN * sizeof(rx_cmd_type_t)];

char echo_response_buf[] =	{'E', 'C', 'H', 'O', 'C', 'H', 'O', 'C', 'O'};
char flash_response_buf[] =	{'F', 'L', 'A', 'S', 'H', 'I', 'N', 'H', 0}; // last byte set to whether will flash
char kill_response_buf[] =	{'K', 'I', 'L', 'L', 'N',  0,   0,   0,  0}; // last 4 bytes for revive timestamp

// init for radio in general
void radio_control_init(void) {
	radio_init();
	rx_command_queue = xQueueCreateStatic(RX_CMD_QUEUE_LEN,
	sizeof(rx_cmd_type_t),
	_rx_command_queue_storage,
	&_rx_command_queue_d);
}

/* Called on a kill command; increases the duration for which we're killed
   for incrementally based on the previous duration. */
void kill_radio_incremental(void) {
	uint32_t prev_radio_kill_timestamp = cache_get_radio_revive_timestamp();
	uint32_t cur_timestamp = get_current_timestamp();
	
	// TODO: we might need another MRAM field;
	// we can't tell just based on the previous radio kill timestamp
	// what we should set it to now (assuming we're increasing 
	// our kill time on each kill command)
}

// listens for RX and handles uplink commands for up to RX_READY_PERIOD_MS
void handle_uplinks(void) {
	// get ready for buffer input and enable RX mode only
	clear_USART_rx_buffer();
	setTXEnable(false);
	setRXEnable(true);
	
	// continue to try and process commands until we're close enough to the
	// time we need to exit RX mode that we need to get prepared
	// (if nothing is on / is added to the queue, we'll simply sleep during that time)
	TickType_t processing_deadline = xTaskGetTickCount() + RX_READY_PERIOD_MS;

	rx_cmd_type_t rx_command;

	while (xTaskGetTickCount() < processing_deadline) {
		// try to receive command from queue, waiting the maximum time we can before the processing deadline
		if (xQueueReceive(rx_command_queue, &rx_command, processing_deadline - xTaskGetTickCount())) {
			switch (rx_command) {
				case CMD_ECHO:
					// just an echo
					if (!radio_is_killed())
						transmit_buf_wait((uint8_t*) echo_response_buf, CMD_RESPONSE_SIZE);
					break;
					
				case CMD_FLASH:
					// write whether we will flash (weren't currently) to last byte
					flash_response_buf[CMD_RESPONSE_SIZE-1] = would_flash_now();
					if (!radio_is_killed())
						transmit_buf_wait((uint8_t*) flash_response_buf, CMD_RESPONSE_SIZE);
					// wait a bit after sending response before actually flashing
					vTaskDelay(FLASH_CMD_PREFLASH_DELAY_MS);
					// if flash task is currently in period between flashes,
					// wake it up and flash. Otherwise, if we're currently flashing,
					// this command does nothing.
					flash_now();
					break;
					
				case CMD_KILL:
					// TODO: let kill command be activated while we're killed??
					kill_radio_incremental(); // TODO: see function
					if (!radio_is_killed()) {
						uint32_t revive_time = cache_get_radio_revive_timestamp();
						memcpy(kill_response_buf + 1, &revive_time, 4);
						transmit_buf_wait((uint8_t*) kill_response_buf, CMD_RESPONSE_SIZE);
					}
					break;
					
				case CMD_NONE:
					// nothing received, wait for more
					break;
			}
		}
		// keep trying if nothing received
	}
}

/************************************************************************/
/* DATA TRANSMISSION FUNCTIONS                                          */
/************************************************************************/
// define buffers here to keep them LOCAL
uint8_t cur_data_buf[MSG_CUR_DATA_LEN];
uint8_t msg_buffer[MSG_BUFFER_SIZE];

// references to what message type each of the buffers is designated for right now
msg_data_type_t slot_1_msg_type = DEFAULT_MSG_TYPE_SLOT_1;
msg_data_type_t slot_2_msg_type = DEFAULT_MSG_TYPE_SLOT_2;
msg_data_type_t slot_3_msg_type = DEFAULT_MSG_TYPE_SLOT_3;
msg_data_type_t slot_4_msg_type = DEFAULT_MSG_TYPE_SLOT_4;

// "linked list" where each index (msg type #) corresponds to the next
// highest priority packet for transmission
msg_data_type_t msg_type_priority_list[NUM_MSG_TYPE]; // see transmit_task for initialization

// for testing purposes, allow access to cur_data_buf
uint8_t* _get_cur_data_buf(void) {
	return cur_data_buf;
}

/*
	Determines the next highest priority message type that is available to transmit, 
	according to our priority list, relative to the starting_msg_type.
	Returns the first transmittable such message type, or -1 if none are.
*/
msg_data_type_t get_next_highest_pri_msg_type(msg_data_type_t starting_msg_type) {
	configASSERT(starting_msg_type != LOW_POWER_DATA);
	msg_data_type_t new_msg_type = starting_msg_type;
	do {
		// get the next highest priority message type from a
		// linked-list like construct (will wrap around)
		new_msg_type = msg_type_priority_list[new_msg_type];
		
		equistack* msg_type_equistack = get_msg_type_equistack(new_msg_type);
		// note: it would only be NULL if somehow a msg_data_type_t bit got corrupted
		if (msg_type_equistack != NULL && msg_type_equistack->cur_size > 0) {
			return new_msg_type;
		}
	} while (new_msg_type != starting_msg_type);
	
	// if we wrapped around, none were transmittable (we won't transmit)
	return (msg_data_type_t) -1;
}

/** 
 * Determines whether the message type default_msg_type is valid to transmit,
 * and if not, returns a substitute message type based on a priority chain.
 * For example, if the satellite is in LOW_POWER mode, this always
 * returns LOW_POWER_DATA, and if the default message type is IDLE_DATA but
 * that stack is empty, it chooses the next highest priority message type.
 * NOTE: this function could lock all the equistack mutexes to be certain of sizes,
 * but it's not necessary (because they only grow)
 */
msg_data_type_t determine_single_msg_to_transmit(msg_data_type_t default_msg_type) {
	// special case for LOW_POWER mode;
	// only transmit LOW_POWER packets unless the equistack is empty
	// (it doesn't matter whether they're transmitted)
	if (low_power_active()) {
		equistack* low_power_equistack = get_msg_type_equistack(LOW_POWER_DATA);
		// note: it would only be NULL if somehow a msg_data_type_t bit got corrupted
		if (low_power_equistack != NULL && low_power_equistack->cur_size > 0) {
			return LOW_POWER_DATA;
		}
		return -1;
	}
	
	// if the equistack for the default message type is empty, check the next highest
	// priority packet type (and the next, until we either find one or find that 
	// none are available)
	equistack* default_msg_type_equistack = get_msg_type_equistack(default_msg_type);
	// note: it would only be NULL if somehow a msg_data_type_t bit got corrupted
	if (default_msg_type_equistack != NULL && default_msg_type_equistack->cur_size > 0) {
		return default_msg_type;
	} else {
		return get_next_highest_pri_msg_type(default_msg_type);
	}
}

/**
 * Determines what message types (in four slots) to transmit, based on what data we have.
 * Returns whether anything should be transmitted.
 */
bool determine_data_to_transmit(void) {
	// start out with the slot defaults and change them (based on priorities) if none are available
	slot_1_msg_type = determine_single_msg_to_transmit(DEFAULT_MSG_TYPE_SLOT_1);
	slot_2_msg_type = determine_single_msg_to_transmit(DEFAULT_MSG_TYPE_SLOT_2);
	slot_3_msg_type = determine_single_msg_to_transmit(DEFAULT_MSG_TYPE_SLOT_3);
	slot_4_msg_type = determine_single_msg_to_transmit(DEFAULT_MSG_TYPE_SLOT_4);
	
	// if no possible msg types were available for any (this only can occur if
	// all relevant equistacks are empty), don't transmit
	// NOTE: it should be the case that they ALL match unless the situation 
	// somehow manages to change in the course of the above lines
	if (slot_1_msg_type == (msg_data_type_t) -1
		|| slot_2_msg_type == (msg_data_type_t) -1
		|| slot_3_msg_type == (msg_data_type_t) -1
		|| slot_4_msg_type == (msg_data_type_t) -1) {
		configASSERT(slot_1_msg_type == slot_2_msg_type 
					&& slot_2_msg_type == slot_3_msg_type 
					&& slot_3_msg_type == slot_4_msg_type); // all match
		return false;
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
	
	// get timestamp that we'll use on each packet transmitted during this sequence
	uint32_t start_transmission_timestamp = get_current_timestamp();

	// collect current data that we'll use for each packet transmitted during this sequence
	read_current_data(cur_data_buf, start_transmission_timestamp);

	// write first packet to be ready to transmit (before taking mutex)
	write_packet(msg_buffer, slot_1_msg_type, start_transmission_timestamp, cur_data_buf);
	
	// actually send buffer over USART to radio for transmission
	// wait here between calls to give buffer
	if (xSemaphoreTake(critical_action_mutex, CRITICAL_MUTEX_WAIT_TIME_TICKS)) 
	{
		// note start time so we can make sure our packet timing
		// is consistent no matter how long it takes to package the message
		TickType_t prev_transmit_start_time = xTaskGetTickCount();
		
		// slot 1 transmit
		transmit_buf_wait(msg_buffer, MSG_SIZE);
		// between transmitting the first packet and starting the next one,
		// re-package the buffer with the message type of the second slot,
		// and then delay any remaining time using RTOS to be timing-consistent
		write_packet(msg_buffer, slot_2_msg_type, start_transmission_timestamp, cur_data_buf);
		vTaskDelayUntil(&prev_transmit_start_time, TIME_BTWN_MSGS_MS / portTICK_PERIOD_MS);
		// TODO: check that packaging doesn't take longer than TIME_BTWN_MSGS_MS (it shouldn't)
		
		// slot 2 transmit
		transmit_buf_wait(msg_buffer, MSG_SIZE);
		write_packet(msg_buffer, slot_3_msg_type, start_transmission_timestamp, cur_data_buf);
		vTaskDelayUntil(&prev_transmit_start_time, TIME_BTWN_MSGS_MS / portTICK_PERIOD_MS);
		
		// slot 3 transmit
		transmit_buf_wait(msg_buffer, MSG_SIZE);
		write_packet(msg_buffer, slot_4_msg_type, start_transmission_timestamp, cur_data_buf);
		vTaskDelayUntil(&prev_transmit_start_time, TIME_BTWN_MSGS_MS / portTICK_PERIOD_MS);
		
		// slot 4 transmit
		transmit_buf_wait(msg_buffer, MSG_SIZE);
		// we don't need to write a new packet because we just transmitted the last one
		// also, no need to delay
		
		xSemaphoreGive(critical_action_mutex);
	} else {
		log_error(ELOC_RADIO, ECODE_CRIT_ACTION_MUTEX_TIMEOUT, true);
	}

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
	TickType_t prev_wake_time = xTaskGetTickCount();
	
	// initialize message priority "linked list" by matching each message type
	// (besides low power) with the next highest priority message type
	msg_type_priority_list[IDLE_DATA] =			FLASH_CMP_DATA; // lowest priority
	msg_type_priority_list[FLASH_CMP_DATA] =	ATTITUDE_DATA;
	msg_type_priority_list[ATTITUDE_DATA] =		FLASH_DATA;
	msg_type_priority_list[FLASH_DATA] =		IDLE_DATA;		// highest priority (wrap around)

	init_task_state(TRANSMIT_TASK); // suspend or run on boot

	for( ;; )
	{	
		/************************************************************************/
		/* STAGE ILLUSTRATION:                                                  
		  t				 t+x_1			   t+x_2	  t+x_3						t+TRANSMIT_TASK_FREQ
		  |-set cmd mode-|-read radio temp-|-transmit-|-rx-|-sleep (power off)--|
		  where:
			x_1 represents the command mode time requirements
			x_2 represents the above plus the maximum (timeout) time for reading radio temp
			x_3 represents the above plus the RX window time
			TRANSMIT_TASK_FREQ represents the precise, RTOS-ensured frequency of the cycle */
		/************************************************************************/
		
		/* block for any leftover time (done first by convention with RTOS and on startup) */
		/* note this time changes with sat state */
		if (low_power_active()) {
			vTaskDelayUntil(&prev_wake_time, TRANSMIT_TASK_LESS_FREQ / portTICK_PERIOD_MS);
		} else {
			vTaskDelayUntil(&prev_wake_time, TRANSMIT_TASK_FREQ / portTICK_PERIOD_MS);
		}
	
		// report to watchdog
		report_task_running(TRANSMIT_TASK);

		/* enter command mode and commence radio temp reading stage */
		read_radio_temp_mode();
		
		// report to watchdog (again)
		report_task_running(TRANSMIT_TASK);
		
		/* if the radio isn't killed, enter transmission stage */
		if (!radio_is_killed()) {
			attempt_transmission();
		}
		
		// report to watchdog (again)
		report_task_running(TRANSMIT_TASK);

		/* enable rx mode on radio and wait for any incoming transmissions */
		handle_uplinks();
		
		/* shut down and block for any leftover time in next loop */
		setRadioState(false, true);
		// TODO: actually confirm??
		
		// report to watchdog (again)
		report_task_running(TRANSMIT_TASK);
	}
	// delete this task if it ever breaks out
	vTaskDelete( NULL );
}

/************************************************************************/
/* UTILITY                                                              */
/************************************************************************/
void debug_print_msg_types(void) {
	const char* msg = "\nSent messages: %s %s %s %s\n";
	const char* type1 = get_msg_type_str(slot_1_msg_type);
	const char* type2 = get_msg_type_str(slot_2_msg_type);
	const char* type3 = get_msg_type_str(slot_3_msg_type);
	const char* type4 = get_msg_type_str(slot_4_msg_type);
	
	print(msg, type1, type2, type3, type4);
}
