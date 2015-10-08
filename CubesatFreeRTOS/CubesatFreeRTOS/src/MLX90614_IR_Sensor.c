/*
 * MLX9061_IR_Sensor.h
 *
 * Created: 30.09.2015 21:51
 *  Author: Arun
 */

#include <MLX90614_IR_Sensor.h>

/* Read data over I2C from IR Sensor
 *
 * buf: The 16-bit (2-byte) buffer pointer
 * addr: The memory address to read from (address constants defined in header)
 */

/** NOTE: THIS IS NOT TESTED **/
void MLX90614_read(uint8_t* buf, uint8_t addr) {
  struct i2c_master_packet read_packet = {
		.address     = MLX90614_READ_ADDR,
		.data_length = 2,
		.data        = buf,
		.ten_bit_address = false,
		.high_speed      = false,
		.hs_master_code  = 0x0,
	};
	
	uint8_t write_buffer[2] = {
		0x00, addr
	};
	
	struct i2c_master_packet write_packet = {
		.address     = MLX90614_WRITE_ADDR,
		.data_length = 2,
		.data        = write_buffer,
		.ten_bit_address = false,
		.high_speed      = false,
		.hs_master_code  = 0x0,
	};
	
	i2c_write_command(&write_packet);
	i2c_read_command(&read_packet);
}

void MLX90614_write(uint8_t* buf) {

}

