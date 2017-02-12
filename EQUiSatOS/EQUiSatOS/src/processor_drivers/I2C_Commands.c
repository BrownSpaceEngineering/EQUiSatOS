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
void configure_i2c_master(Sercom* sercom, i2c_func _i2c_read_func, i2c_func _i2c_write_func,  i2c_func _i2c_write_no_stop_func)
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
	
	//sets up internal functions to allow for emulated i2c functionality
	used_i2c_read_func = _i2c_read_func;
	used_i2c_write_func = _i2c_write_func;
	used_i2c_write_no_stop_func = _i2c_write_no_stop_func;
}

/*
	Configure I2C with standard settings and standard functions
*/

void configure_i2c_standard(Sercom* sercom)
{
	configure_i2c_master(sercom,i2c_read_command,i2c_write_command,i2c_write_command_nostop);
}

/*
	Given a pointer to a packet, perform a read over I2C following the information
	detailed in the packet
*/
void i2c_read_command(struct i2c_master_packet* packet_address)
{
	i2c_master_read_packet_wait(&i2c_master_instance, packet_address);
}

/*
	Given a pointer to a packet, perform a write over I2C in standard fashion
*/
void i2c_write_command(struct i2c_master_packet* packet_address)
{
	i2c_master_write_packet_wait(&i2c_master_instance, packet_address);
}

/*
	Given a pointer to a packet, perform a write over I2C without stopping
*/
void i2c_write_command_nostop(struct i2c_master_packet* packet_address)
{
	i2c_master_write_packet_wait_no_stop(&i2c_master_instance, packet_address);
}

/*
	Write data of length len to address address on the i2c bus
*/
void writeDataToAddress(uint8_t* data, uint8_t len, uint8_t address, bool should_stop){
	struct i2c_master_packet write_packet = {
		.address     = address,
		.data_length = len,
		.data        = data,
		.ten_bit_address = false,
		.high_speed      = false,
		.hs_master_code  = 0x0,
	};
	
	if(should_stop){
		(used_i2c_write_func)(&write_packet);
	}else{
		(used_i2c_write_no_stop_func)(&write_packet);
	}
}

/*
	Read data of length len into buffer buffer from address address at location memoryLocation on the i2c bus, using the corresponding stopping function
*/
void readFromAddressAndMemoryLocation(uint8_t* buffer, uint8_t len, uint8_t address, uint8_t memoryLocation, bool should_stop){
	struct i2c_master_packet read_packet = {
		.address     = address,
		.data_length = len,
		.data        = buffer,
		.ten_bit_address = false,
		.high_speed      = false,
		.hs_master_code  = 0x0,
	};
	
	uint8_t data[] = {memoryLocation};
	
	writeDataToAddress(data,1,address,should_stop);
	(used_i2c_read_func)(&read_packet);
}
