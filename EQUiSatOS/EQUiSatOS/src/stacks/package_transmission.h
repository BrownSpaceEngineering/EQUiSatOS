/*
 * package_transmission.h
 *
 * Created: 12/6/2016 8:47:29 PM
 *  Author: jleiken
 */ 


#ifndef PACKAGE_TRANSMISSION_H_
#define PACKAGE_TRANSMISSION_H_

#include <global.h>
#include "../rtos_tasks/rtos_tasks.h"

/************************************************************************/
/* MESSAGE FORMAT CONSTANTS												*/
/* See https://docs.google.com/spreadsheets/d/1sHQNTC5f5sg6j5DD4OKjuQykpIM3z16uetWT9YuB9PQ	*/
/* for all details and these values										*/
/************************************************************************/
#define CHECKSUM					1
// various sizes in bytes
#define MSG_BUFFER_SIZE				1200
#define MSG_PREAMBLE_LENGTH			12
#define MSG_HEADER_LENGTH			100

// Set number of packets for each packet type
#define ERROR_PACKETS				10
#define ATTITUDE_DATA_PACKETS		10
#define TRANSMIT_DATA_PACKETS		10
#define FLASH_DATA_PACKETS			10

// size of each packet
#define ERROR_PACKET_SIZE			1
#define ATTITUDE_DATA_PACKET_SIZE	94
#define TRANSMIT_DATA_PACKET_SIZE	14
#define FLASH_DATA_PACKET_SIZE		52

// start points for sections
#define START_PREAMBLE		4
#define START_HEADER		12
#define START_ERRORS		128 // = ERROR_PACKETS * 
#define START_DATA			178

// methods
void init_msg_buffer(void);
char* get_msg_buffer(void);
//uint8_t package_msg_arr(void *header, uint8_t *errors, uint8_t error_len, void *data, uint8_t data_len);

#endif /* PACKAGE_TRANSMISSION_H_ */