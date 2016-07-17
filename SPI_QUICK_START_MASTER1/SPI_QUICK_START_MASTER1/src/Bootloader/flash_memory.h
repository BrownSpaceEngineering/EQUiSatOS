/*
 * flash_memory.h
 *
 * Created: 4/10/2016 3:51:29 PM
 *  Author: Gustavo
 */ 


#ifndef FLASH_MEMORY_H_
#define FLASH_MEMORY_H_

#include <nvm.h>


/**
	Function that takes an array(data->binary, size->num_bytes) and writes it in the flash memory
	starting from the initial address.
**/
void save_binary_into_flash(uint8_t* binary, uint32_t num_bytes, uint32_t initial_address);

#endif /* FLASH_MEMORY_H_ */