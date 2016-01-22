#include <MLX90614_IR_Sensor.h>

void read_MLX90614(uint8_t* buf, uint8_t addr) {
	
	uint8_t write_buffer[1] = {
		addr
	};
	
	struct i2c_master_packet write_packet = {
		.address     = MLX90614_I2CADDR,
		.data_length = 1,
		.data        = write_buffer,
		.ten_bit_address = false,
		.high_speed      = false,
		.hs_master_code  = 0x0,
	};
	
	struct i2c_master_packet read_packet = {
		.address     = MLX90614_I2CADDR,
		.data_length = 2,
		.data        = buf,
		.ten_bit_address = false,
		.high_speed      = false,
		.hs_master_code  = 0x0,
	};
	
	i2c_write_command_no_stop(&write_packet);
	i2c_read_command(&read_packet);
}

float readTemp(uint8_t addr) {
	uint8_t read_buffer[2] = {
		0x0,0x0
	};
	read_MLX90614(read_buffer,addr);
	uint16_t res = read_buffer[0] | (((uint16_t)read_buffer[1]) << 8);
	float temp = (float) res;
	temp *= .02;
	temp -= 273.15;
	return temp;
}

float MLX90614_readObjectTempC() {
	return readTemp(MLX90614_TOBJ1);
}


float MLX90614_readAmbientTempC() {
	return readTemp(MLX90614_TA);
}