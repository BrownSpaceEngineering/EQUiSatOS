/*
 * flash_memory.c
 *
 * Created: 4/27/2016 8:26:39 PM
 *  Author: Gustavo
 */ 

#include "flash_memory.h"

/* initializer function for NVM flash memory; must be called before flash_mem_write_bytes */
void flash_mem_init(void) {
	struct nvm_config config_nvm;
	nvm_get_config_defaults(&config_nvm);
	config_nvm.manual_page_write = false;
	nvm_set_config(&config_nvm);
}

/*
	Function to write a number of bytes of data to the NVM flash memory.
	IMPORTANT NOTE: the start_address MUST be a multiple of the page size, namely NVMCTRL_PAGE_SIZE,
	or the data will be written starting at the nearest (lowest) start of a page.
*/
void flash_mem_write_bytes(uint8_t* data, uint32_t num_bytes, uint32_t start_address) {
	enum status_code error_code;
	uint8_t* cur_data_ptr = data;

	// assert start_address is a multiple of the page size (for now)
	if (start_address % NVMCTRL_PAGE_SIZE != 0) {
		while(true) {}; // tsk, tsk, tsk
	}
	
	// write the data in buckets of NVMCTRL_PAGE_SIZE (they're actually written as whole rows, 
	// but we chose pages because the write/unlock(?) commands must be run on pages)
	for (uint32_t addr = start_address; addr < start_address + num_bytes; addr += NVMCTRL_PAGE_SIZE) {
		// make sure that the area of memory we're overwriting is unlocked (wait if NVM busy)
		do {
			error_code = nvm_execute_command(NVM_COMMAND_UNLOCK_REGION, addr, 0);
		} while (error_code == STATUS_BUSY);
		// NOTE: this above command is asynchronous (NVM commands are), but the nvm_update_buffer 
		// called below will wait until it's not busy to complete, so this is fine

		// insert data to be written (it won't actually be written yet), one page at a time
		// (note if the data being written happens to be less than the page size, 
		// only write that amount of data (the other amount will be retained))
		// side note: this function will read out and rewrite the entire row, but requires
		// that we only update a page at a time (see the implementation), which is kinda annoying...)
		// NOTE: offset + length must be less than the page size
		error_code = nvm_update_buffer(addr, cur_data_ptr, 0, min(NVMCTRL_PAGE_SIZE, num_bytes));
		
		// send write command to actually write data to PAGE 
		// NOTE: the NVM controller should be free after the above function, 
		// but we wait on STATUS_BUSY anyways
		do {
			error_code = nvm_execute_command(NVM_COMMAND_WRITE_PAGE, addr, 0);
		} while (error_code == STATUS_BUSY);
		// NOTE: this function is asynchronous, so any function called soon after this should wait
		// on STATUS_BUSY (see above)
		
		cur_data_ptr += NVMCTRL_PAGE_SIZE;
	}
}