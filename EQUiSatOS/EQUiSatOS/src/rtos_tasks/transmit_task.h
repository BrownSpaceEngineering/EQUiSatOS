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
#define RX_READY_PERIOD_MS			2000

// timing constants
#define MAX_CMD_MODE_RECOVERY_TIME_MS		(100 + WARM_RESET_WAIT_AFTER_MS + WARM_RESET_REBOOT_TIME + MAX_RADIO_CMD_TIME)
#define STATE_CHANGE_MONITOR_DELAY_TICKS	15

uint16_t get_radio_temp_cached(void);

#endif /* TRANSMIT_TASK_H_ */