#include <MLX90614_IR_Sensor.h>

void MLX90614_setSMBusAddr(uint8_t device, uint8_t addr) {

}

uint16_t MLX90614_getSMBusAddr(uint8_t device) {
  uint16_t data = 0xFFFF;

  uint8_t read_buf[2] = {0x0, 0x0}; // MSB, LSB
  MLX90614_read(device, MLX90614_EEPROM_SMBUS, read_buf);
  data = *((uint16_t *)read_buf); // Derefence 2 bytes, MSB first

  return data;
}

/* Read data from the MLX90614.
    - SMBus_addr: The SMBus address of the MLX to read from
    - ram_addr: memory address to read
    - buf: destination buffer */
void MLX90614_read(uint8_t SMBus_addr, uint8_t opcode, uint8_t* buf) {
	
	struct i2c_master_packet write_packet = {
		.address     = SMBus_addr,
		.data_length = 1,
		.data        = &opcode,
		.ten_bit_address = false,
		.high_speed      = false,
		.hs_master_code  = 0x0,
	};
	
	struct i2c_master_packet read_packet = {
		.address     = SMBus_addr,
		.data_length = 2,
		.data        = buf,
		.ten_bit_address = false,
		.high_speed      = false,
		.hs_master_code  = 0x0,
	};
	
	i2c_write_command_no_stop(&write_packet);
	i2c_read_command(&read_packet);
}

/* Read the raw IR data from the MLX90614.
    - SMBus_addr: The SMBus address of the MLX to read from
    - channel: MLX90614 memory address to read (i.e. raw IR channel 1 or 2)
   Returns: Raw IR data for given channel address */
uint16_t MLX90614_readRawIRData(uint8_t SMBus_addr, uint8_t channel) {
	uint16_t data = 0xFFFF;

	if (channel == MLX90614_RAWIR1 || channel == MLX90614_RAWIR2) {
		uint8_t read_buf[2] = {0x0, 0x0}; // MSB, LSB
		MLX90614_read(SMBus_addr, channel, read_buf);
		data = *((uint16_t *)read_buf); // Cast pointer to dereference 2 bytes, MSB first
	}

	return data;
}

/* Read the temp data (TODO: what exactly does this do?)
    - SMBus_addr: The SMBus address of the MLX to read from
    - ram_addr: memory address to read
   Return temperature in degrees C (?) */
float MLX90614_readTemp(uint8_t SMBus_addr, uint8_t ram_addr) {
	uint8_t read_buffer[2] = {
		0x0,0x0
	};
	MLX90614_read(SMBus_addr, ram_addr, read_buffer);
	uint16_t res = read_buffer[0] | (((uint16_t)read_buffer[1]) << 8);
	float temp = (float) res;
	temp *= .02;
	temp -= 273.15;
	return temp;
}

float MLX90614_readObjectTempC(uint8_t SMBus_addr) {
	return MLX90614_readTemp(SMBus_addr, MLX90614_TOBJ1);
}


float MLX90614_readAmbientTempC(uint8_t SMBus_addr) {
	return MLX90614_readTemp(SMBus_addr, MLX90614_TA);
}
