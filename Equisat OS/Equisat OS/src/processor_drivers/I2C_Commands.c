/*
 * I2C_Commands.c
 *
 * Created: 4/26/2015 2:45:48 PM
 *  Author: Daniel
 */ 

#include "I2C_Commands.h"

/*
	Configures I2C connection with standard settings
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
	printf("I2C master init status: %d\r\n", init_status);
	while(init_status != STATUS_OK) {
		init_status = i2c_master_init(&i2c_master_instance, sercom, &config_i2c_master);
		printf("I2C master init error status: %d\r\n", init_status);
	}
	//! [init_module]

	//! [enable_module]
	i2c_master_enable(&i2c_master_instance);
	//! [enable_module]
}

/*
	Given a pointer to a packet, perform a write over I2C following the information
	detailed in the packet.
	
	Modified to take a boolean to determine stop/nostop instead of passing function pointers
*/
void i2c_writer_helper(struct i2c_master_packet* packet_address, bool should_stop)
{
	if(should_stop) {
		i2c_master_write_packet_wait(&i2c_master_instance, packet_address);
	} else {
		i2c_master_write_packet_wait_no_stop(&i2c_master_instance, packet_address);
	}
}

/*
	Given a pointer to a packet, perform a write over I2C following the information
	detailed in the packet
*/
void i2c_write_command(struct i2c_master_packet* packet_address){
  i2c_writer_helper(packet_address, true);
}

/*
	Given a pointer to a packet, perform a write over I2C following the information
	detailed in the packet
*/
void i2c_write_command_no_stop(struct i2c_master_packet* packet_address){
  i2c_writer_helper(packet_address, false);
}

/*
	Given a pointer to a packet, perform a read over I2C following the information
	detailed in the packet
*/
void i2c_read_command(struct i2c_master_packet* packet_address){
	uint16_t timeout = 0;
	while ((i2c_master_read_packet_wait(&i2c_master_instance, packet_address)) != STATUS_OK) {
		if (timeout++ == TIMEOUT) {
			break;
			printf("timeout");
		}
	}
}

