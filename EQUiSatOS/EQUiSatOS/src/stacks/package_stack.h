/*
 * package_stack.h
 *
 * Created: 12/6/2016 8:47:29 PM
 *  Author: jleiken
 */ 


#ifndef PACKAGE_STACK_H_
#define PACKAGE_STACK_H_

#include <asf.h>
#include "equistacks.h"
#include "../runnable_configurations/rtos_tasks.h"

void init_buffer(void);
char* get_buffer(void);
uint8_t package_arr(void *header, uint8_t *errors, uint8_t error_len, void *data, uint8_t data_len);
#define CHECKSUM 1
#define BUFFER_SIZE 1200
#define HEADER_LENGTH 100


#endif /* PACKAGE_STACK_H_ */