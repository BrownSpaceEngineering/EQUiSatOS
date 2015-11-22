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
	
	uint8_t write_buffer[1] = {
		addr
	};
	
	uint8_t write_addr = MLX90614_WRITE_ADDR << 1;
	
	struct i2c_master_packet write_packet = {
		.address     = write_addr,
		.data_length = 1,
		.data        = write_buffer,
		.ten_bit_address = false,
		.high_speed      = false,
		.hs_master_code  = 0x0,
	};
	
	struct i2c_master_packet read_packet = {
		.address     = MLX90614_READ_ADDR,
		.data_length = 2,
		.data        = buf,
		.ten_bit_address = false,
		.high_speed      = false,
		.hs_master_code  = 0x0,
	};
	
	i2c_write_command(&write_packet);
	i2c_read_command(&read_packet);
}


float MLX90614_read_temperature() {
	uint8_t temp_buf[2];
	printf("%02x, %02x\r\n", temp_buf[0], temp_buf[1]);
	MLX90614_read(&temp_buf, TA_ADDR);
	printf("%02x, %02x\r\n", temp_buf[0], temp_buf[1]);
	return calcTemperature((temp_buf[1] << 8) | temp_buf[0]);
}

float calcTemperature(int16_t rawTemp) {
	float t = rawTemp;
	printf("%f\r\n", t);
	return (t * 0.02 - 273.15) * 9.0 / 5.0 + 32;
}

