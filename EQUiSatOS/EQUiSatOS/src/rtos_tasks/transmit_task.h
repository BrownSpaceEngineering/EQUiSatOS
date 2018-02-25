/*
 * trans
 *
 * Created: 2/3/2018 22:43:47
 *  Author: mcken
 */ 

#ifndef TRANSMIT_TASK_H_
#define TRANSMIT_TASK_H_

#include "rtos_tasks.h"
#include "data_handling/package_transmission.h"
#include "../telemetry/Radio_Commands.h"


// transmission-related constants
#define TIME_BTWN_MSGS_MS			100 // 2x EOT timeout for radio
#define EXPECTED_TRANSMISSION_TIME	(3*(TRANSMIT_TIME_MS(MSG_SIZE) + TIME_BTWN_MSGS_MS))
#define RX_READY_PERIOD_MS			2000

// packet sequencing
// TODO: is there an optimal?
#define DEFAULT_MSG_TYPE_SLOT_1		IDLE_DATA
#define DEFAULT_MSG_TYPE_SLOT_2		ATTITUDE_DATA
#define DEFAULT_MSG_TYPE_SLOT_3		FLASH_DATA
#define DEFAULT_MSG_TYPE_SLOT_4		FLASH_CMP_DATA

// timing constants
#define MAX_CMD_MODE_RECOVERY_TIME_MS		(100 + WARM_RESET_WAIT_AFTER_MS + WARM_RESET_REBOOT_TIME + MAX_RADIO_CMD_TIME)
#define TEMP_RESPONSE_TIME_MS				300
#define STATE_CHANGE_MONITOR_DELAY_TICKS	15

uint16_t get_radio_temp_cached(void);
uint8_t* _get_cur_data_buf(void);

#endif /* TRANSMIT_TASK_H_ */