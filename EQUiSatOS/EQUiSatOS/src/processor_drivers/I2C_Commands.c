/*
 * I2C_Commands.c
 *
 * Created: 4/26/2015 2:45:48 PM
 *  Author: Daniel
 */ 

#include "I2C_Commands.h"

/*
	Configures I2C connection with standard settings and custom functions
*/
void configure_i2c_master(Sercom* sercom)
{
	/* Initialize config structure and software module. */
	//! [init_conf]
	i2c_master_get_config_defaults(&config_i2c_master);
	//! [init_conf]

	/* Change buffer timeout to something longer. */
	//! [conf_change]
	config_i2c_master.buffer_timeout = TIMEOUT;
	//! [conf_change]

	/* Initialize and enable device with config. */
	//! [init_module]
	int init_status = i2c_master_init(&i2c_master_instance, sercom, &config_i2c_master);
	//printf("I2C master init status: %d\r\n", init_status);
	while(init_status != STATUS_OK) {
		init_status = i2c_master_init(&i2c_master_instance, sercom, &config_i2c_master);
		//printf("I2C master init error status: %d\r\n", init_status);
	}
	//! [init_module]

	//! [enable_module]
	i2c_master_enable(&i2c_master_instance);
	//! [enable_module]
}

/*
	Configure I2C with standard settings and standard functions
*/

void configure_i2c_standard(Sercom* sercom)
{
	configure_i2c_master(sercom);
}

/* 
	Sends a stop condition on the I2C bus
 */
void i2c_send_stop(void) {
	i2c_master_send_stop(&i2c_master_instance);
}

/*
	Given a pointer to a packet, perform a read over I2C following the information
	detailed in the packet
*/
enum status_code i2c_read_command(struct i2c_master_packet* packet_address)
{
	return i2c_master_read_packet_wait(&i2c_master_instance, packet_address);
	//i2c_master_read_packet_wait_no_nack(&i2c_master_instance, packet_address);
}

/*
	Given a pointer to a packet, perform a read over I2C following the information
	detailed in the packet without stopping
*/
enum status_code i2c_read_command_nostop(struct i2c_master_packet* packet_address)
{
	return i2c_master_read_packet_wait_no_stop(&i2c_master_instance, packet_address);
}

/*
	Given a pointer to a packet, perform a write over I2C in standard fashion
*/
enum status_code i2c_write_command(struct i2c_master_packet* packet_address)
{
	return i2c_master_write_packet_wait(&i2c_master_instance, packet_address);
}

/*
	Given a pointer to a packet, perform a write over I2C without stopping
*/
enum status_code i2c_write_command_nostop(struct i2c_master_packet* packet_address)
{
	return i2c_master_write_packet_wait_no_stop(&i2c_master_instance, packet_address);
}

/*
	Write data of length len to address address on the i2c bus
*/
enum status_code writeDataToAddress(uint8_t* data, uint8_t len, uint8_t address, bool should_stop){
	struct i2c_master_packet write_packet = {
		.address     = address,
		.data_length = len,
		.data        = data,
		.ten_bit_address = false,
		.high_speed      = false,
		.hs_master_code  = 0x0,
	};
	
	if(should_stop){
		return i2c_write_command(&write_packet);
	} else {
		return i2c_write_command_nostop(&write_packet);
	}
}

/*
	Wrapper to write to a specific memory location (subAddress) at address
	author: Jarod Boone
*/ 
enum status_code writeDataToAddressSub(uint8_t* data, uint8_t len, uint8_t address, uint8_t* subAddress, bool should_stop){
	enum status_code code, return_code = STATUS_OK; 
	code = writeDataToAddress(subAddress,1,address,should_stop); 
	
	if ((code & 0x0F) != 0){ //check if status code is ok
		return code;
	} 
	
	code = writeDataToAddress(data,len,address,should_stop);
	
	if ((code & 0x0F) != 0){ //check if status code is ok
		return code;
	}
	
	return return_code;
};

/*
	Read data of length len into buffer buffer from address address at location memoryLocation on the i2c bus, using the corresponding stopping function
*/
enum status_code readFromAddressAndMemoryLocation(uint8_t* buffer, uint8_t len, uint8_t address, uint8_t memoryLocation, bool should_stop){
	uint8_t data[] = {memoryLocation};
	
	enum status_code statc = writeDataToAddress(data,1,address,should_stop);
	// checks if the error is STATUS_CATEGORY_OK
	if ((statc & 0xf0) != 0) {
		return statc;
	}
	return readFromAddress(buffer,len,address,true);
}

/*
	Read data of length len into buffer buffer from address address on the i2c bus, using the corresponding stopping function
*/
enum status_code readFromAddress(uint8_t* buffer, uint8_t len, uint8_t address, bool should_stop){
	struct i2c_master_packet read_packet = {
		.address     = address,
		.data_length = len,
		.data        = buffer,
		.ten_bit_address = false,
		.high_speed      = false,
		.hs_master_code  = 0x0,
	};
	
	if(should_stop){
		return i2c_read_command(&read_packet);
	} else {
		return i2c_read_command_nostop(&read_packet);
	}
}
