/*
 * transmit_task.h
 *
 * Created: 2/3/2018 22:43:47
 *  Author: mcken
 */ 

#ifndef TRANSMIT_TASK_H_
#define TRANSMIT_TASK_H_

#include "rtos_tasks.h"
#include "data_handling/package_transmission.h"

// transmission-related constants
#define TIME_BTWN_MSGS_MS			100
#define EXPECTED_TRANSMISSION_TIME	(3*(TRANSMIT_TIME_MS(MSG_SIZE) + TIME_BTWN_MSGS_MS))

/************************************************************************/
/* STRUCTURES FOR SENDING COMMANDS TO RADIO                             */
/************************************************************************/
// timing constants
#define MAX_CMD_MODE_RECOVERY_TIME_MS		(100 + WARM_RESET_WAIT_AFTER_MS + WARM_RESET_REBOOT_TIME + MAX_RADIO_CMD_TIME)
#define STATE_CHANGE_MONITOR_DELAY_TICKS	15

// possible radio command types (other properties of request determined by these)
enum radio_command_type {
	POWER_ON,
	POWER_OFF,
	COLD_RESET,
	WARM_RESET,
	GET_TEMPERATURE
};

typedef struct radio_command_t {
	enum radio_command_type type;		// type of radio command
	void *input_arg;					// input data argument (if necessary)
	void *rx_data_dest;					// location where any received data will be written
										// (quantity of data is based on command type)
	bool *rx_data_ready;				// pointer to boolean flag that will be set to true
										// after valid rx_data is written to rx_data_dest
										// (allows busy looping)
} radio_command_t;

// queue on which to send radio_command_t's to be fulfilled
#define RADIO_CMD_QUEUE_LEN			15 // if anyone tries to add more, they'll get an error
// internals
StaticQueue_t _radio_command_queue_d;
uint8_t _radio_command_queue_storage[RADIO_CMD_QUEUE_LEN * sizeof(radio_command_t)];
// queue handle
QueueHandle_t radio_command_queue;

void radio_control_init(void);
bool submit_radio_command(enum radio_command_type type,
	void *input_arg, void *rx_data_dest, bool *rx_data_ready, TickType_t full_wait_time_ms);

#endif /* TRANSMIT_TASK_H_ */