/*
 * transmit_task.c
 *
 * Created: 9/21/2017 20:39:34
 *  Author: mcken
 */

#include "transmit_task.h"

/************************************************************************/
/* Data transmission buffers                                            */
/************************************************************************/
// define buffers here to keep them LOCAL
uint8_t cur_data_buf[MSG_CUR_DATA_LEN];
uint8_t msg_buffer[MSG_BUFFER_SIZE];

/************************************************************************/
/* RADIO CONTROL FUNCTIONS                                              */
/************************************************************************/
// temporary version of radio temp; set periodically (right before radio transmit)
uint16_t radio_temp_cached = 0;

uint16_t get_radio_temp_cached(void) {
	return radio_temp_cached;
}

// returns we've received a kill command and that command is currently active.
static bool is_radio_killed(void) {
	// note: the cache is the most rad-tolerant area of the memory,
	// so we always grab the value from there
	return cache_get_radio_revive_timestamp() >= get_current_timestamp();
}

static void read_radio_temp_mode(void) {
	// power on radio initially
	setRadioState(true, false);
	// set command mode to allow sending commands
	vTaskDelay(SET_CMD_MODE_WAIT_BEFORE_MS / portTICK_PERIOD_MS);
	setTXEnable(true);
	setRXEnable(true);
	set_command_mode(false); // don't delay, we'll take care of it
	vTaskDelay(SET_CMD_MODE_WAIT_AFTER_MS / portTICK_PERIOD_MS);

	clear_USART_rx_buffer();
	XDL_prepare_get_temp();	
	usart_send_string(radio_send_buffer);
	
	vTaskDelay(TEMP_RESPONSE_TIME_MS / portTICK_PERIOD_MS);
	if (check_checksum(radio_receive_buffer+1, 3, radio_receive_buffer[4])) {
		radio_temp_cached = (radio_receive_buffer[2] << 8) | radio_receive_buffer[3];
		log_if_out_of_bounds(radio_temp_cached, S_RAD_TEMP, ELOC_RADIO_TEMP, true);
	} else {
		log_error(ELOC_RADIO_TEMP, ECODE_TIMEOUT, false);
	}
	setRXEnable(false);
	setTXEnable(false);
}

/************************************************************************/
/* UPLINK FUNCTIONS                                                     */
/************************************************************************/
// queue internals (put here because #includes)
StaticQueue_t _rx_command_queue_d;
uint8_t _rx_command_queue_storage[RX_CMD_QUEUE_LEN * sizeof(rx_cmd_type_t)];

// init for radio in general
void radio_control_init(void) {
	radio_init();
	rx_command_queue = xQueueCreateStatic(RX_CMD_QUEUE_LEN,
		sizeof(rx_cmd_type_t),
		_rx_command_queue_storage,
		&_rx_command_queue_d);
	memset(cur_data_buf, 0, sizeof(cur_data_buf));
}

//Called on a kill command; sets the duration for which the radio is killed   
static void kill_radio_for_time(rx_cmd_type_t cmd) {	
	uint32_t revive_time = get_current_timestamp();
	switch(cmd) {
		case CMD_KILL_3DAYS:
			revive_time += RADIO_KILL_DUR_3DAYS_S;
			log_error(ELOC_RADIO_UPLINK, ECODE_UPLINK_KILL3DAYS, false);
			break;
		case CMD_KILL_WEEK:
			revive_time += RADIO_KILL_DUR_WEEK_S;
			log_error(ELOC_RADIO_UPLINK, ECODE_UPLINK_KILL1WEEK, false);
			break;
		case CMD_KILL_FOREVER: // :'(
			log_error(ELOC_RADIO_UPLINK, ECODE_UPLINK_KILLFOREVER, false);
			revive_time = 0xFFFFFFFF;
			break;		
		default:
			log_error(ELOC_RADIO_UPLINK, ECODE_UNEXPECTED_CASE, false);
	}
	set_radio_revive_timestamp(revive_time);	
}

//revives radio from being killed if revive command is received
static void revive_radio() {	
	set_radio_revive_timestamp(0);
	log_error(ELOC_RADIO_UPLINK, ECODE_UPLINK_REVIVED, false);
}

// listens for RX and handles uplink commands for up to RX_READY_PERIOD_MS
static void handle_uplinks(void) {
	// get ready for buffer input and enable RX mode only
	clear_USART_rx_buffer();
	setTXEnable(false);
	setRXEnable(true);
	
	// continue to try and process commands until we're close enough to the
	// time we need to exit RX mode that we need to get prepared
	// (if nothing is on / is added to the queue, we'll simply sleep during that time)
	TickType_t processing_deadline = xTaskGetTickCount() + (RX_READY_PERIOD_MS / portTICK_PERIOD_MS);

	rx_cmd_type_t rx_command;

	// NOTE: if we get suspended for a while and we're delayed more than the processing deadline,
	// AND THEN to the point where the tick count overflows, this loop will infinite (long) loop, so add an additional
	// condition that the timestamp is reasonably close to the processing deadline
	while (processing_deadline - (RX_READY_PERIOD_MS / portTICK_PERIOD_MS) <= xTaskGetTickCount() 
			&& xTaskGetTickCount() < processing_deadline) {
		// try to receive command from queue, waiting the maximum time we can before the processing deadline
		if (xQueueReceive(rx_command_queue, &rx_command, processing_deadline - xTaskGetTickCount())) {
			bool is_killed = is_radio_killed();
			switch (rx_command) {
				case CMD_ECHO:
					// just an echo
					if (!is_killed)
						for (int i = 0; i < 3; i++) {
							vTaskDelay(PRE_REPLY_DELAY_MS  / portTICK_PERIOD_MS);
							transmit_buf_wait((uint8_t*) echo_response_buf, CMD_RESPONSE_SIZE);
						}
					break;
					
				case CMD_FLASH:
					// write whether we will flash (weren't currently) to last byte
					flash_response_buf[CMD_RESPONSE_SIZE-1] = would_flash_now();
					if (!is_killed)
						for (int i = 0; i < 3; i++) {
							vTaskDelay(PRE_REPLY_DELAY_MS  / portTICK_PERIOD_MS);
							transmit_buf_wait((uint8_t*) flash_response_buf, CMD_RESPONSE_SIZE);
						}
					// wait a bit after sending response before actually flashing
					vTaskDelay(FLASH_CMD_PREFLASH_DELAY_MS  / portTICK_PERIOD_MS);
					// if flash task is currently in period between flashes,
					// wake it up and flash. Otherwise, if we're currently flashing,
					// this command does nothing.
					flash_now();
					break;
				case CMD_REBOOT:
					if (!is_killed)
						for (int i = 0; i < 3; i++) {
							vTaskDelay(PRE_REPLY_DELAY_MS  / portTICK_PERIOD_MS);
							transmit_buf_wait((uint8_t*) reboot_response_buf, CMD_RESPONSE_SIZE);
						}						
					vTaskDelay(REBOOT_CMD_DELAY_MS / portTICK_PERIOD_MS);
					log_error(ELOC_RADIO_UPLINK, ECODE_UPLINK_REBOOT, false);
					write_state_to_storage_emergency(false);
					system_reset();
					break;
				case CMD_KILL_3DAYS:
				case CMD_KILL_WEEK:
				case CMD_KILL_FOREVER:																			
					if (!is_killed) {
						kill_radio_for_time(rx_command);
						uint32_t revive_time = cache_get_radio_revive_timestamp();						
						memcpy(kill_response_buf + 5, &revive_time, 4);
						for (int i = 0; i < 3; i++) {
							vTaskDelay(PRE_REPLY_DELAY_MS  / portTICK_PERIOD_MS);
							transmit_buf_wait((uint8_t*) kill_response_buf, CMD_RESPONSE_SIZE);
						}
					}
					break;
				case CMD_REVIVE: // :)
					revive_radio();
					for (int i = 0; i < 3; i++) {
						vTaskDelay(PRE_REPLY_DELAY_MS  / portTICK_PERIOD_MS);
						transmit_buf_wait((uint8_t*) revive_response_buf, CMD_RESPONSE_SIZE);
					}
					break;
				case CMD_FLASH_KILL:
					flash_kill();
					for (int i = 0; i < 3; i++) {
						vTaskDelay(PRE_REPLY_DELAY_MS  / portTICK_PERIOD_MS);
						transmit_buf_wait((uint8_t*) flash_kill_response_buf, CMD_RESPONSE_SIZE);
					}
					break;
				case CMD_FLASH_REVIVE:
					flash_revive();
					for (int i = 0; i < 3; i++) {
						vTaskDelay(PRE_REPLY_DELAY_MS  / portTICK_PERIOD_MS);
						transmit_buf_wait((uint8_t*) flash_revive_response_buf, CMD_RESPONSE_SIZE);
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
static msg_data_type_t get_next_highest_pri_msg_type(msg_data_type_t starting_msg_type) {
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
	
	// if we wrapped around, none were transmittable, so just transmit the default
	return starting_msg_type;
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
static msg_data_type_t determine_single_msg_to_transmit(msg_data_type_t default_msg_type) {
	// special case for LOW_POWER mode; only transmit LOW_POWER packets
	if (low_power_active()) {
		return LOW_POWER_DATA;
	}
	
	// if the equistack for the default message type is empty, check the next highest
	// priority packet type (and the next, until we either find one or find that 
	// none are available)
	equistack* default_msg_type_equistack = get_msg_type_equistack(default_msg_type);
	// note: it would only be NULL if somehow a msg_data_type_t bit got corrupted
	if (default_msg_type_equistack != NULL && default_msg_type_equistack->cur_size > 0) {
		return default_msg_type;
	} else {
		return get_next_highest_pri_msg_type(default_msg_type); // might say not transmit if all empty
	}
}

/**
 * Determines what message types (in four slots) to transmit, based on what data we have.
 * Returns whether anything should be transmitted.
 */
static bool determine_data_to_transmit(void) {
	// start out with the slot defaults and change them (based on priorities) if none are available
	slot_1_msg_type = determine_single_msg_to_transmit(DEFAULT_MSG_TYPE_SLOT_1);
	slot_2_msg_type = determine_single_msg_to_transmit(DEFAULT_MSG_TYPE_SLOT_2);
	slot_3_msg_type = determine_single_msg_to_transmit(DEFAULT_MSG_TYPE_SLOT_3);
	slot_4_msg_type = determine_single_msg_to_transmit(DEFAULT_MSG_TYPE_SLOT_4);
	return true;
}

void debug_print_msg_types(void);

// attempts to send transmission
static void attempt_transmission(void) {

	if (!determine_data_to_transmit()) {
		// don't transmit right now
		return;
	}
	
	// print a debug message
	debug_print_msg_types();
	
	// get timestamp that we'll use on each packet transmitted during this sequence
	uint32_t start_transmission_timestamp = get_current_timestamp();

	// collect current data that we'll use for each packet transmitted during this sequence
	read_current_data(cur_data_buf, start_transmission_timestamp);

	// write first packet to be ready to transmit (before taking mutex)
	write_packet(msg_buffer, slot_1_msg_type, start_transmission_timestamp, cur_data_buf);
	#ifdef PRINT_HEX_TRANSMISSIONS
		print_sample_transmission(msg_buffer, slot_1_msg_type, start_transmission_timestamp, cur_data_buf);
	#endif
	
	// actually send buffer over USART to radio for transmission
	// wait here between calls to give buffer
	if (xSemaphoreTake(critical_action_mutex, CRITICAL_MUTEX_WAIT_TIME_TICKS)) 
	{
		// turn on IR power to save radio on time during transmission
		bool got_irpow_semaphore = enable_ir_pow_if_necessary();
		
		// note time before transmit so we can try and align transmissions
		TickType_t prev_transmit_start_time = xTaskGetTickCount();
		
		// slot 1 transmit
		transmit_buf_wait(msg_buffer, MSG_SIZE);
		// between finishing transmitting the first packet and starting the next one,
		// re-package the buffer with the message type of the second slot,
		// and then delay any remaining time using RTOS to be timing-consistent
		write_packet(msg_buffer, slot_2_msg_type, start_transmission_timestamp, cur_data_buf);
		#ifdef PRINT_HEX_TRANSMISSIONS
			print_sample_transmission(msg_buffer, slot_2_msg_type, start_transmission_timestamp, cur_data_buf);
		#endif
		// delay until a specified time after the expected transmission time plus some bufer
		vTaskDelayUntil(&prev_transmit_start_time, TOTAL_PACKET_TRANS_TIME_MS / portTICK_PERIOD_MS);
		
		// slot 2 transmit
		transmit_buf_wait(msg_buffer, MSG_SIZE);
		
		// in low power, only transmit two packets
		if (!low_power_active()) {
			configASSERT(slot_3_msg_type != LOW_POWER_DATA && slot_4_msg_type != LOW_POWER_DATA);
			
			write_packet(msg_buffer, slot_3_msg_type, start_transmission_timestamp, cur_data_buf);
			#ifdef PRINT_HEX_TRANSMISSIONS
				print_sample_transmission(msg_buffer, slot_3_msg_type, start_transmission_timestamp, cur_data_buf);
			#endif
			vTaskDelayUntil(&prev_transmit_start_time, TOTAL_PACKET_TRANS_TIME_MS / portTICK_PERIOD_MS);
			
			// slot 3 transmit
			transmit_buf_wait(msg_buffer, MSG_SIZE);
			write_packet(msg_buffer, slot_4_msg_type, start_transmission_timestamp, cur_data_buf);
			#ifdef PRINT_HEX_TRANSMISSIONS
				print_sample_transmission(msg_buffer, slot_4_msg_type, start_transmission_timestamp, cur_data_buf);
			#endif	
			vTaskDelayUntil(&prev_transmit_start_time, TOTAL_PACKET_TRANS_TIME_MS / portTICK_PERIOD_MS);
			
			// slot 4 transmit
			transmit_buf_wait(msg_buffer, MSG_SIZE);
		}
		// we don't need to write a new packet because we just transmitted the last one
		// no delay after finish
		
		disable_ir_pow_if_necessary(got_irpow_semaphore);
		xSemaphoreGive(critical_action_mutex);
	} else {
		log_error(ELOC_RADIO, ECODE_CRIT_ACTION_MUTEX_TIMEOUT, false);
	}
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
	
	//used to only read one out of every 3 iterations
	uint8_t temp_read_count = 1;

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
	
		// report to watchdog
		report_task_running(TRANSMIT_TASK);		
		
		// make sure radio is powered on either for transmission or handling uplinks
		setRadioState(true, false);
		
		/* if the radio isn't killed, enter transmission stage */
		if (!is_radio_killed()) {
			attempt_transmission();
		}
		
		// report to watchdog (again)
		report_task_running(TRANSMIT_TASK);

		/* enable rx mode on radio and wait for any incoming transmissions */
		handle_uplinks();
		
		// report to watchdog (again)
		report_task_running(TRANSMIT_TASK);
		
		if (temp_read_count == 3) {
			temp_read_count = 1;
			/* enter command mode and commence radio temp reading stage */
			read_radio_temp_mode();	
		} else {
			temp_read_count++;
		}
		
		/* shut down and block for any leftover time in next loop */
		setRadioState(false, false);
		
		// report to watchdog (again)
		report_task_running(TRANSMIT_TASK);
		
		/* block for any leftover time */
		/* note this time changes with sat state */
		if (low_power_active()) {
			vTaskDelayUntil(&prev_wake_time, TRANSMIT_TASK_LESS_FREQ / portTICK_PERIOD_MS);
			} else {
			vTaskDelayUntil(&prev_wake_time, TRANSMIT_TASK_FREQ / portTICK_PERIOD_MS);
		}
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
