/*
 * Data_Heap.c
 *
 * Created: 11/20/2016 3:43:58 PM
 *  Author: mcken
 */ 

#include "data_heap.h"

data_t_heap *create_data_t_heap(size_t slot_size, int16_t num_slots)
{ 
	// TODO: maybe suspend all tasks while doing this to make sure they're no malloc calls?
	data_t_heap* heap = pvPortMalloc(sizeof(data_t_heap));
	heap->slot_size = slot_size;
	heap->num_slots = num_slots;
	heap->first_free_index = 0;
	heap->root_pointer = pvPortMalloc(slot_size * num_slots);
		
	// TODO: do we have memory allocated for this whole array of pointers?
	for (int i = 0; i < num_slots; i++)
	{
		clear_existing_data(get_ptr_at_index(i, heap), slot_size);
		
		// mark all as available
		heap->allocated_slots[i] = false;
	}
	
	return heap;
}

void free_data_t_heap(data_t_heap* heap)
{	
	vPortFree(heap->root_pointer);
	vPortFree(heap);
}

void *data_t_malloc(data_t_heap* heap)
{
	// check that a valid free_index exists
	if (heap->first_free_index < heap->num_slots)
	{
		// create a pointer to the first free spot by using the distance from the origin pointer
		void* first_free_ptr = get_ptr_at_index(heap->first_free_index, heap);
		
		heap->allocated_slots[heap->first_free_index] = true;
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
	if (heap->root_pointer <= ptr && ptr <= get_ptr_at_index(heap->num_slots - 1, heap))
	{
		// normalize all memory here to zeros
		clear_existing_data(ptr, heap->slot_size);
	
		int slot_index = (ptr - heap->root_pointer) / heap->slot_size; // may be one off....
	
		// mark as available
		heap->allocated_slots[slot_index] = false;
	
		// adjust first_free_index if necessary
		if (slot_index < heap->first_free_index)
			heap->first_free_index = slot_index;
	}
}

/* Helper functions */
void* get_ptr_at_index(int16_t index, data_t_heap* heap)
{
	return heap->root_pointer + index * heap->slot_size;
}

void clear_existing_data(void* ptr, size_t slot_size)
{
	// convert the pointer to a char pointer to iterate over bytes
	char* byte_ptr = (char*) ptr;
	for (int16_t i = 0; i < slot_size; i++)
	{
		// set memory value to zero
		*(byte_ptr + i) = 0;
	}
}

void update_first_free_index(data_t_heap* heap)
{
	// start from old first_free_index (we assume it's been allocated)
	for (int16_t i = heap->first_free_index; i < heap->num_slots; i++)
	{
		// note first free spot found
		if (heap->allocated_slots[i] == false)
		{
			heap->first_free_index = i;
			return;
		}
	}	
	// if no free memory found, first_free_index is invalid (off the end)
	heap->first_free_index = heap->num_slots;
}