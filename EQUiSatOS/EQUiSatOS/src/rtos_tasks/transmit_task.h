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
#include "../sensor_drivers/sensor_read_commands.h"


// transmission-related constants
#define TIME_BTWN_MSGS_MS			300 // > 2x EOT timeout for radio
#define TOTAL_PACKET_TRANS_TIME_MS	(TOTAL_TRANSMIT_TIME_MS(MSG_SIZE) + TIME_BTWN_MSGS_MS - IR_WAKE_DELAY_MS) // we wake IR before so it doesn't require that time
#define EXPECTED_TRANSMISSION_TIME	(3*TOTAL_PACKET_TRANS_TIME_MS)
#define RX_READY_PERIOD_MS			1000
#define PRE_REPLY_DELAY_MS			700
#define FLASH_CMD_PREFLASH_DELAY_MS 1500
#define REBOOT_CMD_DELAY_MS			2000

#define RADIO_KILL_DUR_3DAYS_S		259200		// 3 days
#define RADIO_KILL_DUR_WEEK_S		604800		// 7 days

// packet sequencing
// TODO: is there an optimal order?
#define DEFAULT_MSG_TYPE_SLOT_1		IDLE_DATA
#define DEFAULT_MSG_TYPE_SLOT_2		ATTITUDE_DATA
#define DEFAULT_MSG_TYPE_SLOT_3		FLASH_DATA
#define DEFAULT_MSG_TYPE_SLOT_4		FLASH_CMP_DATA

// timing constants
#define MAX_CMD_MODE_RECOVERY_TIME_MS		(100 + WARM_RESET_WAIT_AFTER_MS + WARM_RESET_REBOOT_TIME + MAX_RADIO_CMD_TIME)
#define TEMP_RESPONSE_TIME_MS				400
#define STATE_CHANGE_MONITOR_DELAY_TICKS	15

// queue on which to receive rx_cmd_type_t's from UART interrupt to be processed
#define RX_CMD_QUEUE_LEN			3
// queue handle
QueueHandle_t rx_command_queue;

void radio_control_init(void);
uint16_t get_radio_temp_cached(void);
uint8_t* _get_cur_data_buf(void);

#endif /* TRANSMIT_TASK_H_ */