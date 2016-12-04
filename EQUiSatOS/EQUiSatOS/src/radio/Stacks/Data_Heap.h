/*
 * Data_Heap.h
 *
 * Created: 11/20/2016 3:20:34 PM
 *  Author: mcken
 */ 

#ifndef DATA_HEAP_H_
#define DATA_HEAP_H_

#define NONE_UNALLOC_FREE_INDEX -1;

#include <asf.h>

// TODO: it may be better to simply allocate the heaps with some given first size,
// and then only use part of it when a data_t_heap is actually created

typedef struct data_t_heap 
{
	size_t slot_size;			// size of each slot in bytes
	int16_t num_slots;			// number of slot_size sized slots
	int16_t first_free_index;	// index in slots of the first unallocated slot
	void** slots;				// array of pointers representing each slot
} data_t_heap;

data_t_heap* create_data_t_heap(size_t slot_size, int16_t num_slots);
void free_data_t_heap(data_t_heap* heap);

void *data_t_malloc(data_t_heap* heap);
void data_t_free(void* ptr, data_t_heap* heap);

/* Helper functions */
void clear_existing_data(void* ptr, size_t slot_size);
void update_first_free_index(data_t_heap* heap);

#endif /* DATA_HEAP_H_ */