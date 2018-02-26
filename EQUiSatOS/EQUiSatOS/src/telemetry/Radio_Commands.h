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
#define RADIO_BAUD_BYTES			1200
#define TRANSMIT_TIME_MS(bytes)		((1000 * bytes) / RADIO_BAUD_BYTES)
#define SET_CMD_MODE_WAIT_BEFORE_MS	200
#define SET_CMD_MODE_WAIT_AFTER_MS	300
#define MAX_RADIO_CMD_TIME			200 // 20 ms inner delay * 20
#define WARM_RESET_REBOOT_TIME		1000
#define WARM_RESET_WAIT_AFTER_MS	100 // TODO: necessary / what value?

#define LEN_GROUND_CALLSIGN			4
extern char ground_callsign_buf[LEN_GROUND_CALLSIGN];
#define LEN_ECHOBUF					4
extern char echo_buf[LEN_ECHOBUF];
#define LEN_KILLBUF					4
extern char kill_buf[LEN_KILLBUF];
#define LEN_FLASHBUF				5
extern char flash_buf[LEN_FLASHBUF];

bool check_checksum(char* data, int dataLen, uint8_t actualChecksum);

void set_command_mode(bool delay);
bool XDL_prepare_get_temp();
bool warm_reset(void);
void cold_reset(void);

void radio_init(void);
void transmit_buf_wait(const uint8_t* buf, size_t size);
void setRadioState(bool enable, bool confirm);
void setTXEnable(bool enable);
void setRXEnable(bool enable);
void set3V6Power(bool on);
void setRadioPower(bool on);

typedef enum {	CMD_NONE = 0,
				CMD_ECHO = 1,
				CMD_KILL = 2,
				CMD_FLASH = 3
				} rx_cmd_type_t;

rx_cmd_type_t check_rx_received(void);
bool check_if_rx_matches(char* buf, uint8_t len, uint8_t rx_buf_index);

#endif /* RADIO_COMMANDS_H_ */

