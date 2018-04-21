/*
 * Radio_Commands.h
 *
 * Created: 9/20/2016 9:50:03 PM
 *  Author: Tyler
 */


#ifndef RADIO_COMMANDS_H_
#define RADIO_COMMANDS_H_

#include "../processor_drivers/USART_Commands.h"
#include "../sensor_drivers/sensor_read_commands.h"

// constants
#define RADIO_BAUD_BYTES				1200
#define TRANSMIT_TIME_MS(bytes)			((1000 * bytes) / RADIO_BAUD_BYTES)
#define TRANSMIT_CURRENT_RISE_TIME_MS	250
#define TRANSMIT_CURRENT_FALL_TIME_MS	100
#define TOTAL_TRANSMIT_TIME_MS(bytes)	(IR_WAKE_DELAY_MS + TRANSMIT_CURRENT_RISE_TIME_MS + TRANSMIT_TIME_MS(bytes)) // see transmit_buf_wait
#define SET_CMD_MODE_WAIT_BEFORE_MS		1000
#define SET_CMD_MODE_WAIT_AFTER_MS		500
#define MAX_RADIO_CMD_TIME				200 // 20 ms inner delay * 20
#define WARM_RESET_REBOOT_TIME			1000
#define WARM_RESET_WAIT_AFTER_MS		500 // TODO: necessary / what value?

#define LEN_GROUND_CALLSIGN			4
extern char ground_callsign_buf[LEN_GROUND_CALLSIGN];
#define LEN_UPLINK_BUF				2
extern char echo_buf[LEN_UPLINK_BUF];
extern char kill_3days_buf[LEN_UPLINK_BUF];
extern char kill_week_buf[LEN_UPLINK_BUF];
extern char kill_forever_buf[LEN_UPLINK_BUF];
extern char flash_buf[LEN_UPLINK_BUF];
extern char reboot_buf[LEN_UPLINK_BUF];
#define CMD_RESPONSE_SIZE			9
extern char echo_response_buf[CMD_RESPONSE_SIZE];
extern char flash_response_buf[CMD_RESPONSE_SIZE];
extern char reboot_response_buf[CMD_RESPONSE_SIZE];
extern char kill_response_buf[CMD_RESPONSE_SIZE];
extern char revive_response_buf[CMD_RESPONSE_SIZE];
extern char flash_kill_response_buf[CMD_RESPONSE_SIZE];
extern char flash_revive_response_buf[CMD_RESPONSE_SIZE];

bool flash_killed;
void flash_kill(void);
void flash_revive(void);
	
bool check_checksum(uint8_t* data, uint8_t dataLen, uint8_t actualChecksum);

void set_command_mode(bool delay);
void XDL_prepare_get_temp(void);
void warm_reset(void);
void cold_reset(void);

void radio_init(void);
void transmit_buf_wait(const uint8_t* buf, size_t size);
void setRadioState(bool enable, bool confirm);
void setTXEnable(bool enable);
void setRXEnable(bool enable);
void set3V6Power(bool on);
void setRadioPower(bool on);

typedef enum {	
	CMD_NONE,
	CMD_ECHO,
	CMD_REBOOT,
	CMD_FLASH,
	CMD_KILL_3DAYS,
	CMD_KILL_WEEK,
	CMD_KILL_FOREVER,
	CMD_REVIVE,
	CMD_FLASH_KILL,
	CMD_FLASH_REVIVE,
	
} rx_cmd_type_t;

rx_cmd_type_t check_rx_received(void);
bool check_if_rx_matches(char* buf, uint8_t len, uint8_t rx_buf_index);

#endif /* RADIO_COMMANDS_H_ */

