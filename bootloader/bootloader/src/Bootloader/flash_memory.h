/*
 * flash_memory.h
 *
 * Created: 4/10/2016 3:51:29 PM
 *  Author: Gustavo
 */ 


#ifndef FLASH_MEMORY_H_
#define FLASH_MEMORY_H_

#include <nvm.h>

/* initializer function for NVM flash memory; must be called before flash_mem_write_bytes */
void flash_mem_init(void);

/*
	Function to write a number of bytes of data to the NVM flash memory.
	IMPORTANT NOTE: the start_address MUST be a multiple of the page size, namely NVMCTRL_PAGE_SIZE,
	or the data will be written starting at the nearest (lowest) start of a page.
*/
void flash_mem_write_bytes(uint8_t* data, uint32_t num_bytes, uint32_t start_address);

#endif /* FLASH_MEMORY_H_ */