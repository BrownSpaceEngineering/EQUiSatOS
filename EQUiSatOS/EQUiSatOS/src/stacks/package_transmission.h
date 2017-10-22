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

/************************************************************************/
/* MESSAGE FORMAT CONSTANTS												*/
/* See https://docs.google.com/spreadsheets/d/1sHQNTC5f5sg6j5DD4OKjuQykpIM3z16uetWT9YuB9PQ	*/
/* for all details and these values										*/
/************************************************************************/
#define CHECKSUM					1

// various sizes in bytes
#define MSG_BUFFER_SIZE				1201 // = sum of all sections (largest) + 1 for null terminator
#define MSG_PREAMBLE_LENGTH			12
#define MSG_HEADER_LENGTH			36
#define MSG_CORRECTION_LENGTH		16

// start points for sections
#define START_PREAMBLE				0
#define START_HEADER				12
#define START_ERRORS				48
#define START_DATA					68
#define START_CORRECTION			239

// Set number of packets for each packet type
#define ERROR_PACKETS				13
#define IDLE_DATA_PACKETS			7
#define ATTITUDE_DATA_PACKETS		3
#define FLASH_DATA_PACKETS			1
#define FLASH_CMP_PACKETS			8

// size of each packet
#define ERROR_PACKET_SIZE			2
#define IDLE_DATA_PACKET_SIZE		22
#define ATTITUDE_DATA_PACKET_SIZE	52
#define FLASH_DATA_PACKET_SIZE		164
#define FLASH_CMP_PACKET_SIZE		20

// methods
void assert_transmission_constants(void);

void write_preamble(		char* buffer, uint32_t timestamp, uint32_t states, uint8_t data_len);
void write_header(			char* buffer, idle_data_t *idle_data);
void write_errors(			char* buffer, equistack *error_stack);
void write_idle_data(		char* buffer, equistack *idle_stack);
void write_attitude_data(	char* buffer, equistack *attitude_stack);
void write_flash_data(		char* buffer, equistack *flash_stack);
void write_flash_cmp(		char* buffer, equistack *flash_cmp_stack);

void write_bytes_and_shift(char *data, void *input, size_t num_bytes, uint8_t *index);
void write_value_and_shift(char *data, char value, size_t num_bytes, uint8_t *index);

#endif /* PACKAGE_TRANSMISSION_H_ */