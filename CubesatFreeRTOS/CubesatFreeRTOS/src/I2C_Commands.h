/*
 * IncFile1.h
 *
 * Created: 4/26/2015 3:34:57 PM
 *  Author: Daniel
 */ 


#ifndef I2C_COMMANDS_H
#define I2C_COMMANDS_H

#include <asf.h>
#include <conf_uart_serial.h>

#define TIMEOUT 1000

struct i2c_master_module i2c_master_instance;
struct i2c_master_config config_i2c_master;
void configure_i2c_master(void);
void i2c_write_command(struct i2c_master_packet* packet_address);
void i2c_write_command_no_stop(struct i2c_master_packet* packet_address);
void i2c_read_command(struct i2c_master_packet* packet_address);

#endif /* INCFILE1_H_ */
