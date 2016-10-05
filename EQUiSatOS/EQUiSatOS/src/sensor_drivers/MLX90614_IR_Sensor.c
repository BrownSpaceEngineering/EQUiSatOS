#include "MLX90614_IR_Sensor.h"

void MLX90614_init(i2c_func _i2c_write_func, i2c_func _i2c_read_func, i2c_func _i2c_write_no_stop_func) {
	MLX90614_i2c_write_func = _i2c_write_func;
	MLX90614_i2c_write_no_stop_func = _i2c_write_no_stop_func;
	MLX90614_i2c_read_func = _i2c_read_func;
}

/* Read 2 bytes over I2C from an MLX device.
    device_addr: I2C address of the device
    mem_addr: RAM or EEPROM address to read OR'd with its opcode (see datasheet)
    buf: 2-byte destination data buffer
*/
void read_MLX90614(MLXDeviceAddr addr, uint8_t mem_addr, uint8_t* buf) {
	struct i2c_master_packet write_packet = {
		.address     = device_addr,
		.data_length = 1,
		.data        = &mem_addr,
		.ten_bit_address = false,
		.high_speed      = false,
		.hs_master_code  = 0x0,
	};
	
	struct i2c_master_packet read_packet = {
		.address     = device_addr,
		.data_length = 2,
		.data        = buf,
		.ten_bit_address = false,
		.high_speed      = false,
		.hs_master_code  = 0x0,
	};

	(*MLX90614_i2c_write_no_stop_func)(&write_packet);
	(*MLX90614_i2c_read_func)(&read_packet);
}


/* Write a value to an MLX's EEPROM e.g. to modify its SMBus address.
    device_addr: I2C address of the device
    mem_addr: EEPROM address to write to OR'd with its opcode (see datasheet)
    buf: 2-byte source data buffer
*/
void write_MLX90614_eeprom(MLXDeviceAddr addr, uint8_t mem_addr, uint8_t* buf) {
	
	uint8_t crc_buf[3] = {
		mem_addr, buf[0], buf[1]
	};
	
	uint8_t write_buf[4] = {
		mem_addr, buf[0], buf[1], crc(crc_buf,3)
	};
	
	struct i2c_master_packet write_packet = {
		.address     = device_addr,
		.data_length = 4,
		.data        = write_buf,
		.ten_bit_address = false,
		.high_speed      = false,
		.hs_master_code  = 0x0,
	};
	
	(*MLX90614_i2c_write_func)(&write_packet);
}

// reads a 2 byte value which is sorted:
uint16_t MLX90614_read2ByteValue(MLXDeviceAddr addr, uint8_t mem_addr) {
	uint8_t read_buffer[2] = {
		0x0, 0x0
	};
	read_MLX90614(device_addr, mem_addr, read_buffer);
	return read_buffer[0] | (((uint16_t)read_buffer[1]) << 8); // assumes LSB is in read_buffer[0]
}

// Function to read raw IR data from sensor. chan can be IR1 or IR2
uint16_t MLX90614_readRawIRData(MLXDeviceAddr addr, IRChannel chan) {
	return MLX90614_read2ByteValue(device_addr, (uint8_t)chan);
}

// converts a data value from the sensor corresponding to a temperature memory address to a celsius temperature
float dataToTemp(uint16_t data){
	float raw_temp = (float)data; // treat data as a 16-bit float
  return 0.02*raw_temp - 273.15;
}

// Read target temperature in degrees Celsius
// temp_target can be AMBIENT, OBJ1, OBJ2
float MLX90614_readTempC(MLXDeviceAddr addr, IRTempTarget temp_target) {
	uint16_t data = MLX90614_read2ByteValue(device_addr, (uint8_t) temp_target);
	return dataToTemp(data);
}

// sanity check, should always return the same value as device_addr
uint16_t MLX90614_getAddress(MLXDeviceAddr addr) {
	return MLX90614_read2ByteValue(device_addr, MLX90614_SMBUS);
}

// changes device address from current_addr to new_addr. make sure new_addr doesnt conflict with any networked i2c devices
void MLX90614_setAddress(MLXDeviceAddr current_addr, MLXDeviceAddr new_addr) {
	uint16_t mem = MLX90614_getAddress(current_addr);
	uint8_t* raws = (uint8_t*) &mem;
	raws[1] = new_addr;
	write_MLX90614_eeprom(current_addr, MLX90614_SMBUS, raws);
}

