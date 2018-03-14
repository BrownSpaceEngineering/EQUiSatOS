/*
 * IncFile1.h
 *
 * Created: 4/26/2015 3:34:57 PM
 *  Author: Daniel
 */ 


#ifndef I2C_COMMANDS_H
#define I2C_COMMANDS_H

#include <global.h>

#define TIMEOUT 65535

struct i2c_master_module i2c_master_instance;
struct i2c_master_config config_i2c_master;
void configure_i2c_master(Sercom* sercom);
void configure_i2c_standard(Sercom* sercom);
void i2c_send_stop(void);
enum status_code i2c_read_command(struct i2c_master_packet* packet_address);
enum status_code i2c_read_command_nostop(struct i2c_master_packet* packet_address);
enum status_code i2c_write_command(struct i2c_master_packet* packet_address);
enum status_code i2c_write_command_nostop(struct i2c_master_packet* packet_address);

enum status_code writeDataToAddress(uint8_t* data, uint8_t len, uint8_t address, bool should_stop);
enum status_code writeDataToAddressSub(uint8_t* data, uint8_t len, uint8_t address, uint8_t* subAddress, bool should_stop);
enum status_code readFromAddressAndMemoryLocation(uint8_t* buffer, uint8_t len, uint8_t address, uint8_t memoryLocation, bool should_stop);
enum status_code readFromAddress(uint8_t* buffer, uint8_t len, uint8_t address, bool should_stop);

#endif /* INCFILE1_H_ */
