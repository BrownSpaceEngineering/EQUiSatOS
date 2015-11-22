/*
 * I2C_Commands.c
 *
 * Created: 4/26/2015 2:45:48 PM
 *  Author: Daniel
 */ 

#include <I2C_Commands.h>

/*
	Configures I2C connection with standard settings
*/
void configure_i2c_master(void)
{
	/* Initialize config structure and software module. */
	//! [init_conf]
	i2c_master_get_config_defaults(&config_i2c_master);
	//! [init_conf]

	/* Change buffer timeout to something longer. */
	//! [conf_change]
	config_i2c_master.buffer_timeout = 65535;
	//! [conf_change]

	/* Initialize and enable device with config. */
	//! [init_module]
	int init_status = i2c_master_init(&i2c_master_instance, SERCOM2, &config_i2c_master);
	printf("I2C master init status: %d\r\n", init_status);
	while(init_status != STATUS_OK) {
		init_status = i2c_master_init(&i2c_master_instance, SERCOM2, &config_i2c_master);
		printf("I2C master init error status: %d\r\n", init_status);
	}
	//! [init_module]

	//! [enable_module]
	i2c_master_enable(&i2c_master_instance);
	//! [enable_module]
}

/*
	Given a pointer to a packet, perform a write over I2C following the information
	detailed in the packet
*/
void i2c_writer_helper(struct i2c_master_packet* packet_address,
                       enum status_code (*i2c_write)(struct i2c_master_module *const module,
                                                     struct i2c_master_packet *const packet))
{
	uint16_t timeout = 0;
	while (true) {
		int x = i2c_write(&i2c_master_instance, packet_address);
		if (x == STATUS_OK){
			break;
		}
		if(timeout%100 == 0) {
			printf("i2c_master_write_packet_wait status: %d\r\n",x);
		}
		// Increment timeout counter and check if timed out.
		if (timeout++ == TIMEOUT) {
			printf("I2C write timed out.\r\n");
			break;
		}
		printf("i2c_master_write_packet_wait status: %d\r\n",x);
	}
}

/*
	Given a pointer to a packet, perform a write over I2C following the information
	detailed in the packet
*/
void i2c_write_command(struct i2c_master_packet* packet_address){
  i2c_writer_helper(packet_address, i2c_master_write_packet_wait);
}

/*
	Given a pointer to a packet, perform a write over I2C following the information
	detailed in the packet
*/
void i2c_write_command_no_stop(struct i2c_master_packet* packet_address){
  i2c_writer_helper(packet_address, i2c_master_write_packet_wait_no_stop);
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

