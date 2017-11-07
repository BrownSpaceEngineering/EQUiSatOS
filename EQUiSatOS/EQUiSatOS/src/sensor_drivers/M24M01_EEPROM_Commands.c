/*
 * M24M01_EEPROM_Commands.c
 *
 * Created: 11/6/2017 7:25:41 PM
 *  Author: Ryan Izant
 */ 

#include "M24M01_EEPROM_Commands.h"

// Writes a single byte to the EEPROM at the address determined by mem_addr
// Implements "Byte Write" from datasheet
enum status_code writeByteToM24M01(EEPROM_addr_struct mem_addr, uint8_t data){
	// concatenate address from base + memory location MSB
	uint8_t bus_addr = EEPROM_MEM_ADDR + mem_addr.MSB_2; 	
	
	uint8_t toSend[3] = {mem_addr.MSB_8,mem_addr.LSB_8,data};
	enum status_code statc = writeDataToAddress(toSend,3,bus_addr,true);
	
	return statc;
}

// Writes num_bytes number (up to 256) of bytes given by data to the EEPROM with the start address start_mem_addr
// The second byte of data will be written to memory address start_mem_addr+1 etc.s
// Implements "Page Write" from datasheet
enum status_code writeBytesToM24M01(EEPROM_addr_struct start_mem_addr, uint8_t* data, uint8_t num_bytes){
	// concatenate address from base + memory location MSB
	uint8_t bus_addr = EEPROM_MEM_ADDR + start_mem_addr.MSB_2;
	
	uint8_t toSend[num_bytes+2];
	toSend[0] = start_mem_addr.MSB_8;
	toSend[1] = start_mem_addr.LSB_8;
	for(int i=0; i<num_bytes; i++){
		toSend[i+2] = data[i];
	}
	enum status_code statc = writeDataToAddress(toSend,num_bytes+2,bus_addr,true);
	
	return statc;
}

// Reads a single byte at the given mem_addr and returns it in data
// Implements "Random Address Read" from datasheet
enum status_code readByteFromM24M01(EEPROM_addr_struct mem_addr, uint8_t *data){
	// concatenate address from base + memory location MSB
	uint8_t bus_addr = EEPROM_MEM_ADDR + mem_addr.MSB_2; 
	uint8_t toSend[2] = {mem_addr.MSB_8,mem_addr.LSB_8};
	
	enum status_code statc = writeDataToAddress(toSend,2,bus_addr,false);
	
	statc = readFromAddress(data,1,bus_addr,true);	
	return statc;
}

// Reads num_bytes (up to 256) of data from the memory at address start_mem_addr.
// The Nth byte read will be read from the memory location start_mem_addr+N
// Implements "Sequential Random Read" from datasheet
enum status_code readBytesFromM24M01(EEPROM_addr_struct start_mem_addr, uint8_t* data, uint8_t num_bytes){
	// concatenate address from base + memory location MSB
	uint8_t bus_addr = EEPROM_MEM_ADDR + start_mem_addr.MSB_2; 
	uint8_t toSend[2] = {start_mem_addr.MSB_8,start_mem_addr.LSB_8};
	
	enum status_code statc = writeDataToAddress(toSend,2,bus_addr,false);
	
	statc = readFromAddress(data,num_bytes,bus_addr,true);	
	return statc;
}

