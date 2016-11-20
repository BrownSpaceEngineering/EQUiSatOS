/*
 * Data_Heap.c
 *
 * Created: 11/20/2016 3:43:58 PM
 *  Author: mcken
 */ 

#include "Data_Heap.h"

data_t_heap *create_data_t_heap(size_t slot_size, uint16_t num_slots)
{
	data_t_heap* heap = pvPortMalloc(sizeof(data_t_heap));
	heap->slot_size = slot_size;
	heap->num_slots = num_slots;
		
	// TODO: do we have memory allocated for this whole array of pointers?
	for (int i = 0; i < num_slots; i++)
	{
		heap->slots[i] = pvPortMalloc(slot_size);
		// TODO: do we need to do this:
		// clear_existing_data(heap->slots[i], slot_size)
	}
}

void *data_t_malloc(data_t_heap* heap)
{
	//		*first slot pointer*		*less than last slot pointer*		 *increment by slot size*
	//for (void* ptr = heap->slots[0], ptr < heap->num_slots * heap->slot_size; ptr += heap->slot_size)
	
	for (int i = 0; i < heap->num_slots; i++)
	{
		if (heap->slots[i] == NULL)
		{
			// create a pointer to this spot by using the distance from the origin pointer
			// *this assumes that pvPortMalloc DOES NOT span the heap across many locations
			return heap->slots[0] + i * heap->slot_size;
		}
	}
	
	// no free memory available
	return NULL;	
}

void data_t_free(void* ptr, data_t_heap* heap)
{
	// normalize all memory here to zeros
	clear_existing_data(ptr, heap->slot_size);
	
	int slot_index = (ptr - heap->slots[0]) / heap->slot_size; // may be one off....
	
	// mark as available
	heap->slots[slot_index] = NULL;
}

/* Helper functions */
void clear_existing_data(void* ptr, size_t slot_size)
{
	// convert the pointer to a char pointer to iterate over bytes
	char* byte_ptr = (char*) ptr;
	for (int i = byte_ptr; i < slot_size; i++)
	{
		// set memory value to zero
		*byte_ptr = 0;
	}
}