#include "MLX90614_IR_Sensor.h"

void MLX90614_init() {
	setup_pin(true,POWER_PIN); //init low power ir pin
	irPower(true);
}

/*
	Powers on or off the ir sensor.  True is on, false is off.
*/
void irPower(bool trueIsOn){
	set_output(trueIsOn, POWER_PIN);	
};

/*
	returns if the ir is powered on
*/
bool isIROn(){
	return get_output(POWER_PIN);
}

// reads a 2 byte value which is sorted:
uint16_t MLX90614_read2ByteValue(MLXDeviceAddr addr, uint8_t mem_addr) {
	uint8_t read_buffer[2] = {
		0x0, 0x0
	};
	readFromAddressAndMemoryLocation(read_buffer,2,addr,mem_addr,MLX90614_SHOULD_STOP);
	return read_buffer[0] | (((uint16_t)read_buffer[1]) << 8); // assumes LSB is in read_buffer[0]
}

// Function to read raw IR data from sensor. chan can be IR1 or IR2
uint16_t MLX90614_readRawIRData(MLXDeviceAddr addr, IRChannel chan) {
	return MLX90614_read2ByteValue(addr, (uint8_t)chan);
}

// converts a data value from the sensor corresponding to a temperature memory address to a celsius temperature
float dataToTemp(uint16_t data){
	float raw_temp = (float)data; // treat data as a 16-bit float
	return 0.02*raw_temp - 273.15;
}

// Read target temperature in degrees Celsius
// temp_target can be AMBIENT, OBJ1, OBJ2
float MLX90614_readTempC(MLXDeviceAddr addr, IRTempTarget temp_target) {
	uint16_t data = MLX90614_read2ByteValue(addr, (uint8_t) temp_target);
	return dataToTemp(data);
}

// sanity check, should always return the same value as device_addr
uint16_t MLX90614_getAddress(MLXDeviceAddr addr) {
	return MLX90614_read2ByteValue(addr, MLX90614_SMBUS);
}

