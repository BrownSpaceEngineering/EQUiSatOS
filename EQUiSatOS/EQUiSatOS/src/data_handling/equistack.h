/*
 * equistack.h
 *
 * Created: 3/1/2017 8:43:43 PM
 *  Author: mcken
 */


#ifndef EQUISTACK_H_
#define EQUISTACK_H_

#define EQUISTACK_MUTEX_WAIT_TIME_TICKS 500

#include <asf.h> // don't #include global, there are circular dependencies!
#include "task.h"
#include "semphr.h"

typedef struct equistack
{
	int16_t     top_index;
	int16_t     bottom_index;
	int16_t     cur_size;
	int16_t		max_size;
	size_t		data_size;
	SemaphoreHandle_t mutex;
	void*		data;
} equistack;

equistack* equistack_Init(equistack* S, void* data, size_t data_size, uint16_t max_size, SemaphoreHandle_t mutex);
void* equistack_Get(equistack* S, int16_t n);
void* equistack_Get_Unsafe(equistack* S, int16_t n);
void* equistack_Get_From_Bottom(equistack* S, int16_t n);
void* equistack_Get_From_Bottom_Unsafe(equistack* S, int16_t n);
void* equistack_Initial_Stage(equistack* S);
void* equistack_Stage(equistack* S);
void* equistack_Stage_Unsafe(equistack* S);
void* equistack_Stage_from_isr(equistack* S);
void* equistack_Push(equistack* S, void* data);
void* equistack_Push_Unsafe(equistack* S, void* data);
void* equistack_Push_from_isr(equistack* S, void* data);
void __equistack_Clear(equistack* S);
void clear_existing_data(void* ptr, size_t slot_size);

#endif /* EQUISTACK_H_ */
