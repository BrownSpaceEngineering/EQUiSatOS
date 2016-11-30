/*
 * Data_Heap.c
 *
 * Created: 11/20/2016 3:43:58 PM
 *  Author: mcken
 */ 

#include "Data_Heap.h"

data_t_heap *create_data_t_heap(size_t slot_size, int16_t num_slots)
{ 
	// TODO: maybe suspend all tasks while doing this to make sure they're no malloc calls?
	data_t_heap* heap = pvPortMalloc(sizeof(data_t_heap));
	heap->slot_size = slot_size;
	heap->num_slots = num_slots;
	heap->first_free_index = 0;
		
	// TODO: do we have memory allocated for this whole array of pointers?
	for (int i = 0; i < num_slots; i++)
	{
		heap->slots[i] = pvPortMalloc(slot_size); 
		// TODO: do we need to do this:
		// clear_existing_data(heap->slots[i], slot_size)
	}
	
	return heap;
}

void free_data_t_heap(data_t_heap* heap)
{	
	for (int i = 0; i < heap->num_slots; i++)
	{
		vPortFree(heap->slots[i]);
	}
	vPortFree(heap);
}

void *data_t_malloc(data_t_heap* heap)
{
	//		*first slot pointer*		*less than last slot pointer*		 *increment by slot size*
	//for (void* ptr = heap->slots[0], ptr < heap->num_slots * heap->slot_size; ptr += heap->slot_size)
	
	// check that a valid free_index exists
	if (heap->first_free_index < heap->num_slots)
	{
		// create a pointer to the first free spot by using the distance from the origin pointer
		// ****this assumes that pvPortMalloc DOES NOT span the heap across many locations****
		void* first_free_ptr = heap->slots[0] + heap->first_free_index * heap->slot_size;
		
		update_first_free_index(heap);
		return first_free_ptr;
	}
	else
	{
		// no free memory
		return NULL;
	}
}

void data_t_free(void* ptr, data_t_heap* heap)
{
	// normalize all memory here to zeros
	clear_existing_data(ptr, heap->slot_size);
	
	int slot_index = (ptr - heap->slots[0]) / heap->slot_size; // may be one off....
	
	// mark as available
	heap->slots[slot_index] = NULL;
	
	// adjust first_free_index if necessary
	if (slot_index < heap->first_free_index)
		heap->first_free_index = slot_index;
}

/* Helper functions */
void clear_existing_data(void* ptr, size_t slot_size)
{
	// convert the pointer to a char pointer to iterate over bytes
	char* byte_ptr = (char*) ptr;
	for (int16_t i = byte_ptr; i < slot_size; i++)
	{
		// set memory value to zero
		*byte_ptr = 0;
	}
}

void update_first_free_index(data_t_heap* heap)
{
	// start from old first_free_index (we assume it's been allocated)
	for (int16_t i = heap->first_free_index; i < heap->num_slots; i++)
	{
		// note first free spot found
		if (heap->slots[i] == NULL)
		{
			heap->first_free_index = i;
			return;
		}
	}	
	// if no free memory found, first_free_index is invalid (off the end)
	heap->first_free_index = heap->num_slots;
}