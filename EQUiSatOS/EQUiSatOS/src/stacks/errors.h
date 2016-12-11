/*
 * errors.h
 *
 * Created: 12/11/2016 2:19:48 PM
 *  Author: jleiken
 *
 * Errors are stored in this without a wrapper struct, they're simply uint16_t's
 */ 


#ifndef ERRORS_H_
#define ERRORS_H_

#define ERROR_STACK_MAX 10
#include <asf.h>
#include "../runnable_configurations/rtos_task_frequencies.h"

typedef struct error_data_t
{
	uint32_t timestamp;
	uint16_t errors[idle_MAX_READS_PER_LOG / idle_IR_READS_PER_LOG];
} error_data_t;

typedef struct error_Stack
{
	uint16_t*   data[ERROR_STACK_MAX];
	int16_t     size;
	int16_t     top_index;
	int16_t     bottom_index;
} error_Stack;

void error_Stack_Init(error_Stack*);
error_data_t* error_Stack_Top(error_Stack*);
void error_Stack_Push(error_Stack*, error_data_t*);

void free_error_data(error_data_t* es);



#endif /* ERRORS_H_ */