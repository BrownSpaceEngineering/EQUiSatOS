/*
 * flash_memory.c
 *
 * Created: 4/27/2016 8:26:39 PM
 *  Author: Gustavo
 */ 

#include "flash_memory.h"

void configure_nvm(void)
{
	struct nvm_config config_nvm;
	nvm_get_config_defaults(&config_nvm);
	nvm_set_config(&config_nvm);
}

void erase_row(uint32_t initial_address){
	enum status_code error_code;
	do
	{
		error_code = nvm_erase_row(
		64* NVMCTRL_ROW_PAGES * NVMCTRL_PAGE_SIZE);
	} while (error_code == STATUS_BUSY);
}

void copy_array_to_buffer(uint8_t* binary, uint32_t num_bytes, uint8_t* buffer, uint32_t initial_address){
	uint32_t end_of_data = min(NVMCTRL_PAGE_SIZE, num_bytes);
	int i;
	for(i = 0; i < end_of_data; ++i){
		buffer[i] = binary[i];
	}
	for(;i < NVMCTRL_PAGE_SIZE; ++i){
		buffer[i] = 0xff;
	}
}


/**
	Function that takes an array(data->binary, size->num_bytes) and writes it in the flash memory
	starting from the initial address.
	
	It is necessary to erase every page and then copy the data to a buffer(to save it)
	
**/
void save_binary_into_flash(uint8_t* binary, uint32_t num_bytes, uint32_t initial_address){
	enum status_code error_code;

	configure_nvm();
	struct nvm_config config_nvm;
	nvm_get_config_defaults(&config_nvm);
	config_nvm.manual_page_write = false;
	nvm_set_config(&config_nvm);
	
	unsigned char buffer[NVMCTRL_PAGE_SIZE];
	int i;
	for (i = initial_address; i < initial_address + num_bytes; i += NVMCTRL_PAGE_SIZE){
		erase_row(i);
		error_code = nvm_execute_command(NVM_COMMAND_UNLOCK_REGION ,
		i * NVMCTRL_ROW_PAGES * NVMCTRL_PAGE_SIZE, 0);
		
		copy_array_to_buffer(binary, num_bytes, buffer, i);
		nvm_update_buffer(i * NVMCTRL_ROW_PAGES * NVMCTRL_PAGE_SIZE, buffer, 0, NVMCTRL_PAGE_SIZE);
	}	
}


