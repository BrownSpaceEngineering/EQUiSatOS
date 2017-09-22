/*
 * package_transmission.h
 *
 * Created: 12/6/2016 8:47:29 PM
 *  Author: jleiken
 */ 


#ifndef PACKAGE_TRANSMISSION_H_
#define PACKAGE_TRANSMISSION_H_

#include <global.h>
#include "../runnable_configurations/rtos_tasks.h"

/************************************************************************/
/* MESSAGE FORMAT CONSTANTS												*/
/************************************************************************/
#define CHECKSUM 1
#define MSG_BUFFER_SIZE 1200
#define MSG_HEADER_LENGTH 100
#define MSG_DATA_LENGTH 100

#define ATTITUDE_DATA_PACKETS 10
#define RADIO_DATA_PACKETS 10
#define FLASH_DATA_PACKETS 10

void init_msg_buffer(void);
char* get_msg_buffer(void);
uint8_t package_msg_arr(void *header, uint8_t *errors, uint8_t error_len, void *data, uint8_t data_len);

#endif /* PACKAGE_TRANSMISSION_H_ */