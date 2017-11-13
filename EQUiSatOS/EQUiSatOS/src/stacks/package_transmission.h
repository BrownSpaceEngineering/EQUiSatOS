/*
 * package_transmission.h
 *
 * Created: 12/6/2016 8:47:29 PM
 *  Author: jleiken
 */ 

#ifndef PACKAGE_TRANSMISSION_H_
#define PACKAGE_TRANSMISSION_H_

#include <global.h>
#include "rtos_tasks/rtos_tasks.h"
#include "../telemetry/rscode-1.3/ecc.h"

/************************************************************************/
/* MESSAGE FORMAT CONSTANTS												*/
/* See https://docs.google.com/spreadsheets/d/1sHQNTC5f5sg6j5DD4OKjuQykpIM3z16uetWT9YuB9PQ	*/
/* for all details and these values										*/
/************************************************************************/
#define CHECKSUM					1

// various sizes in bytes
#define MSG_BUFFER_SIZE				256 // = sum of all sections (largest) + 1 for null terminator
#define MSG_PREAMBLE_LENGTH			12
#define MSG_CUR_DATA_LEN			36
#define MSG_CORRECTION_LENGTH		16

// start points for sections
#define START_PREAMBLE				0
#define START_CUR_DATA				12
#define START_ERRORS				48
#define START_DATA					68
#define START_PARITY				239

// Set number of packets for each packet type
#define PRIORITY_ERROR_PACKETS		9
#define NORMAL_ERROR_PACKETS		4
#define ERROR_PACKETS				PRIORITY_ERROR_PACKETS + NORMAL_ERROR_PACKETS
#define IDLE_DATA_PACKETS			7
#define ATTITUDE_DATA_PACKETS		3
#define FLASH_DATA_PACKETS			1
#define FLASH_CMP_PACKETS			8

// size of each packet
#define CALLSIGN_SIZE				4
#define ERROR_PACKET_SIZE			3
#define IDLE_DATA_PACKET_SIZE		22
#define ATTITUDE_DATA_PACKET_SIZE	52
#define FLASH_DATA_PACKET_SIZE		164
#define FLASH_CMP_PACKET_SIZE		20

// the time resolution to store error time deltas in;
// we have chosen 300s = 5min because it gives approximately a
// day of errors (1280 mins = 21.33 hours = 13.8 orbits)
#define ERROR_TIME_BUCKET_SIZE		300 // s

// methods
void assert_transmission_constants(void);

void write_preamble(		uint8_t* buffer, uint32_t timestamp, uint8_t states, uint8_t data_len);
void write_current_data(	uint8_t* buffer);
void write_errors(			uint8_t* buffer, equistack* priority_equistack, equistack* normal_equistack, uint32_t timestamp);

void write_idle_data(		uint8_t* buffer, equistack *idle_stack);
void write_attitude_data(	uint8_t* buffer, equistack *attitude_stack);
void write_flash_data(		uint8_t* buffer, equistack *flash_stack);
void write_flash_cmp(		uint8_t* buffer, equistack *flash_cmp_stack);

void write_parity(			uint8_t* buffer);

void write_bytes_and_shift(	uint8_t* data, void *input, size_t num_bytes, uint8_t *index);
void write_value_and_shift(	uint8_t* data, char value, size_t num_bytes, uint8_t *index);

#endif /* PACKAGE_TRANSMISSION_H_ */