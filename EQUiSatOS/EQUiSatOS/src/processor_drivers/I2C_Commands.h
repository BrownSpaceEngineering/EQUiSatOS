/*
 * IncFile1.h
 *
 * Created: 4/26/2015 3:34:57 PM
 *  Author: Daniel
 */ 


#ifndef I2C_COMMANDS_H
#define I2C_COMMANDS_H

#include <asf.h>

#define TIMEOUT 65535

typedef void (*i2c_func)(struct i2c_master_packet*);

struct i2c_master_module i2c_master_instance;
struct i2c_master_config config_i2c_master;
void configure_i2c_master(Sercom* sercom, i2c_func _i2c_read_func, i2c_func _i2c_write_func,  i2c_func _i2c_write_no_stop_func);
void configure_i2c_standard(Sercom* sercom);
void i2c_read_command(struct i2c_master_packet* packet_address);
void i2c_write_command(struct i2c_master_packet* packet_address);
void i2c_write_command_nostop(struct i2c_master_packet* packet_address);

void writeDataToAddress(uint8_t* data, uint8_t len, uint8_t address, bool should_stop);
void readFromAddressAndMemoryLocation(uint8_t* buffer, uint8_t len, uint8_t address, uint8_t memoryLocation, bool should_stop);

i2c_func used_i2c_write_func;
i2c_func used_i2c_read_func;
i2c_func used_i2c_write_no_stop_func;

#endif /* INCFILE1_H_ */
