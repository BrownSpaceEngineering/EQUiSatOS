/*
 * transmit_task.c
 *
 * Created: 9/21/2017 20:39:34
 *  Author: mcken
 */

#include "rtos_tasks.h"
#include "data_handling/package_transmission.h"

// define buffer here to keep LOCAL
uint8_t msg_buffer_1[MSG_BUFFER_SIZE];
uint8_t msg_buffer_2[MSG_BUFFER_SIZE];
uint8_t msg_buffer_3[MSG_BUFFER_SIZE];

// forward declarations
static void write_packet(uint8_t* msg_buffer, int state);
static void write_message_top(uint8_t* msg_buffer, uint8_t num_data, size_t size_data, msg_data_type_t msg_type);

void transmit_task(void *pvParameters)
{
	// initialize xNextWakeTime once
	TickType_t prev_wake_time = xTaskGetTickCount();

	// count of how many times the current packet has been transmitted
	uint8_t current_sequence_transmissions = 0;

	// references to what message type each of the buffers is designated for right now
	msg_data_type_t buffer_1_msg_type = IDLE_DATA;		// 0
	msg_data_type_t buffer_2_msg_type = ATTITUDE_DATA;	// 1
	msg_data_type_t buffer_3_msg_type = FLASH_DATA;		// 2

	init_task_state(TRANSMIT_TASK); // suspend or run on boot

	for( ;; )
	{
		// block for a time based on this task's globally-set frequency
		// (Note: changes to the frequency can be delayed in taking effect by as much as the past frequency...)
		vTaskDelayUntil( &prev_wake_time, TRANSMIT_TASK_FREQ / portTICK_PERIOD_MS);

		// report to watchdog
		report_task_running(TRANSMIT_TASK);

		// only grab new transmission and fill message buffer after we've transmitted this one enough
		if (current_sequence_transmissions >= TRANSMIT_TASK_MSG_REPEATS) {
			current_sequence_transmissions = 0;

			// shift all message types over one, wrapping around
			// TODO: Only do this if there equistack is NOT empty for the corresponding new type
			buffer_1_msg_type = (buffer_1_msg_type + 1) % NUM_MSG_TYPE;
			buffer_2_msg_type = (buffer_2_msg_type + 1) % NUM_MSG_TYPE;
			buffer_3_msg_type = (buffer_3_msg_type + 1) % NUM_MSG_TYPE;

			//write_packet(msg_buffer_1, buffer_1_msg_type);
			//write_packet(msg_buffer_2, buffer_2_msg_type);
			//write_packet(msg_buffer_3, buffer_3_msg_type);
		}

		// actually send buffer over USART to radio for transmission
		// TODO: We may want to wait here between calls to allow for actual data transfer
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

void write_packet(uint8_t* msg_buffer, int state) {
	// based on what state we're in, compile a different message
	switch(state)
	{
		case IDLE_DATA:
			write_message_top(msg_buffer, IDLE_DATA_PACKETS, IDLE_DATA_PACKET_SIZE, IDLE_DATA);
			write_attitude_data(msg_buffer, &idle_readings_equistack);
			break;

		case ATTITUDE_DATA:
			write_message_top(msg_buffer, ATTITUDE_DATA_PACKETS, ATTITUDE_DATA_PACKET_SIZE, ATTITUDE_DATA);
			write_attitude_data(msg_buffer, &attitude_readings_equistack);
			break;

		case FLASH_DATA:
			write_message_top(msg_buffer, FLASH_DATA_PACKETS, FLASH_DATA_PACKET_SIZE, FLASH_DATA);
			write_flash_data(msg_buffer, &flash_readings_equistack);
			break;

		case FLASH_CMP_DATA:
			write_message_top(msg_buffer, FLASH_CMP_DATA_PACKETS, FLASH_CMP_DATA_PACKET_SIZE, FLASH_CMP_DATA);
			write_flash_data(msg_buffer, &flash_cmp_readings_equistack);
			break;
			
		case LOW_POWER_DATA:
			write_message_top(msg_buffer, LOW_POWER_DATA_PACKETS, LOW_POWER_DATA_PACKET_SIZE, LOW_POWER_DATA);
			write_low_power_data(msg_buffer, &low_power_readings_equistack);

		default:
			break;
	};

	write_parity(msg_buffer);
}

void write_message_top(uint8_t* msg_buffer, uint8_t num_data, size_t size_data, msg_data_type_t msg_type) {
	// configure state string
	uint8_t state_string = 0;
	state_string |=  msg_type & 0b11; // two LSB of msg_type (4 types)
	state_string |= (get_sat_state() & 0b111) << 2; // three LSB of satellite state

	uint32_t current_timestamp = get_rtc_count();

	// write preamble
	write_preamble(msg_buffer, current_timestamp, state_string,
		num_data * size_data);

	// read sensors and write current data to buffer; it's not dependent on state
	write_current_data(msg_buffer, current_timestamp);

	// write relevant errors from both error stacks
	write_errors(msg_buffer, &priority_error_equistack, &normal_error_equistack, current_timestamp);
}
