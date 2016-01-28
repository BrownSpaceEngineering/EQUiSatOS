#include <MLX90614_IR_Sensor.h>

void read_MLX90614(uint8_t* buf, uint8_t addr) {
	
	struct i2c_master_packet write_packet = {
		.address     = MLX90614_I2CADDR,
		.data_length = 1,
		.data        = &addr,
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

uint16_t MLX90614_readRawIRData(uint8_t addr) {
	uint16_t data = 0xFFFF;

	if (addr == MLX90614_RAWIR1 || addr == MLX90614_RAWIR2) {
		uint8_t read_buf[2] = {0x0, 0x0}; // MSB, LSB
		read_MLX90614(read_buf, MLX90614_RAWIR1);
		data = *((uint16_t *)read_buf); // Cast pointer to dereference 2 bytes, MSB first
	}

	return data;
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