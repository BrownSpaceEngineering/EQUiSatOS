/*
 * package_transmission.h
 *
 * Created: 12/6/2016 8:47:29 PM
 *  Author: mckenna
 */ 

#ifndef PACKAGE_TRANSMISSION_H_
#define PACKAGE_TRANSMISSION_H_

#include <global.h>
#include "rtos_tasks/rtos_tasks.h"
#include "../telemetry/rscode-1.3/ecc.h"
#include "../data_handling/persistent_storage.h"

/************************************************************************/
/* MESSAGE FORMAT CONSTANTS												*/
/* See https://docs.google.com/spreadsheets/d/1sHQNTC5f5sg6j5DD4OKjuQykpIM3z16uetWT9YuB9PQ	*/
/* for all details and these values										*/
/************************************************************************/
// various section sizes in bytes
#define MSG_PREAMBLE_LENGTH			11
#define MSG_CUR_DATA_LEN			16
#define MSG_DATA_AND_ERRORS_LEN		196 // sum of possible data, error, and padding sections
#define MSG_PARITY_LENGTH			32
#define MSG_SIZE					255
#define MSG_BUFFER_SIZE				(MSG_SIZE + 1) // = sum of all sections (largest) + 1 for null terminator

// start points for sections
#define START_PREAMBLE				0
#define START_CUR_DATA				11
#define START_DATA					27
#define START_PARITY				223

// set number of packets for each packet type
#define IDLE_DATA_PACKETS			7
#define ATTITUDE_DATA_PACKETS		5
#define FLASH_DATA_PACKETS			1
#define FLASH_CMP_DATA_PACKETS		6
#define LOW_POWER_DATA_PACKETS		5

// size of each packet
#define CALLSIGN_SIZE				4
#define ERROR_PACKET_SIZE			3
#define IDLE_DATA_PACKET_SIZE		23
#define ATTITUDE_DATA_PACKET_SIZE	33
#define FLASH_DATA_PACKET_SIZE		151
#define FLASH_CMP_DATA_PACKET_SIZE	25
#define LOW_POWER_DATA_PACKET_SIZE	30

// number of errors in each packet type (truncating is INTENTIONAL)
#define IDLE_DATA_NUM_ERRORS			((MSG_DATA_AND_ERRORS_LEN - IDLE_DATA_PACKETS * IDLE_DATA_PACKET_SIZE) / ERROR_PACKET_SIZE)
#define ATTITUDE_DATA_NUM_ERRORS		((MSG_DATA_AND_ERRORS_LEN - ATTITUDE_DATA_PACKETS * ATTITUDE_DATA_PACKET_SIZE) / ERROR_PACKET_SIZE)
#define FLASH_DATA_NUM_ERRORS			((MSG_DATA_AND_ERRORS_LEN - FLASH_DATA_PACKETS * FLASH_DATA_PACKET_SIZE) / ERROR_PACKET_SIZE)
#define FLASH_CMP_DATA_NUM_ERRORS		((MSG_DATA_AND_ERRORS_LEN - FLASH_CMP_DATA_PACKETS * FLASH_CMP_DATA_PACKET_SIZE) / ERROR_PACKET_SIZE)
#define LOW_POWER_DATA_NUM_ERRORS		((MSG_DATA_AND_ERRORS_LEN - LOW_POWER_DATA_PACKETS * LOW_POWER_DATA_PACKET_SIZE) / ERROR_PACKET_SIZE)
	
// size of padding after each packet (just get it off the spreadsheet, too hard to calc)
#define IDLE_DATA_PADDING_SIZE			2
#define ATTITUDE_DATA_PADDING_SIZE		1
#define FLASH_DATA_PADDING_SIZE			0
#define FLASH_CMP_DATA_PADDING_SIZE		1
#define LOW_POWER_DATA_PADDING_SIZE		1

// the time resolution to store error time deltas in;
// we have chosen 300s = 5min because it gives approximately a
// day of errors (1280 mins = 21.33 hours = 13.8 orbits)
#define ERROR_TIME_BUCKET_SIZE		300 // s

// methods
void assert_transmission_constants(void);

void write_packet(uint8_t* msg_buffer, msg_data_type_t msg_type, uint32_t current_timestamp);

void write_bytes_and_shift(uint8_t *data, uint8_t* buf_index, void *input, size_t num_bytes);
void write_value_and_shift(uint8_t *data, uint8_t* buf_index, char value, size_t num_bytes);

#endif /* PACKAGE_TRANSMISSION_H_ */