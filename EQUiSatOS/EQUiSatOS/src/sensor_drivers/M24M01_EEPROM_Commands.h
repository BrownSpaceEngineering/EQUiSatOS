/*
 * M24M01_EEPROM_Commands.h
 *
 * Created: 11/6/2017 7:26:26 PM
 *  Author: Ryan Izant
 */ 


#ifndef M24M01_EEPROM_COMMANDS_H_
#define M24M01_EEPROM_COMMANDS_H_

#include "../global.h"

//Struct for sending the 18 bit memory address
typedef struct {
		uint8_t LSB_8; //Least significant 8 bits
		uint8_t MSB_8; //Middle 8 bits (A8-A15)
		uint8_t MSB_2; //Most significant 2 bits
	}EEPROM_addr_struct;

// Address for accessing normal memory bits.
/* Least 2 significant bits are A17 and A16 of the address for the memory location
	that you want to read/write from*/
#define EEPROM_MEM_ADDR	0b1010000   
//Address for accessing the "Identification Page"
#define EEPROM_ID_ADDR	0b1011000 

enum status_code writeByteToM24M01(EEPROM_addr_struct mem_addr, uint8_t data);
enum status_code writeBytesToM24M01(EEPROM_addr_struct start_mem_addr, uint8_t* data, uint8_t num_bytes);

enum status_code readByteFromM24M01(EEPROM_addr_struct mem_addr, uint8_t *data);
enum status_code readBytesFromM24M01(EEPROM_addr_struct start_mem_addr, uint8_t* data, uint8_t num_bytes);

#endif /* M24M01_EEPROM_COMMANDS_H_ */