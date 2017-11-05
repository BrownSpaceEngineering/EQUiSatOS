#include "MLX90614_IR_Sensor.h"

void MLX90614_init() {
	setup_pin(true,P_IR_PWR_CMD); //init low power ir pin
	irPower(true);
}

/*
	Powers on or off the ir sensor.  True is on, false is off.
*/
void irPower(bool trueIsOn){
	set_output(trueIsOn, P_IR_PWR_CMD);
};

/*
	returns if the ir is powered on
*/
bool isIROn(){
	return get_output(P_IR_PWR_CMD);
}

// reads a 2 byte value which is sorted:
void MLX90614_read2ByteValue(MLXDeviceAddr addr, uint8_t mem_addr, return_struct_16 rs) {
	uint8_t read_buffer[2] = {
		0x0, 0x0
	};
	rs.return_status = readFromAddressAndMemoryLocation(read_buffer,2,addr,mem_addr,MLX90614_SHOULD_STOP);
	rs.return_value = read_buffer[0] | (((uint16_t)read_buffer[1]) << 8); // assumes LSB is in read_buffer[0]
}

// Function to read raw IR data from sensor. chan can be IR1 or IR2
void MLX90614_readRawIRData(MLXDeviceAddr addr, IRChannel chan, return_struct_16 rs) {
	MLX90614_read2ByteValue(addr, (uint8_t)chan, rs);
}

void MLX90614_read_all_obj(MLXDeviceAddr addr, return_struct_16 rs){
	uint16_t val[2];
	uint8_t read_buffer[2] = {
		0x0, 0x0
	};

	MLX90614_read2ByteValue(addr, (uint8_t)OBJ1, rs);
	enum status_code read_obj1 =  rs.return_status;

	if ((read_obj1 & 0x0F) != 0){
		rs.return_status = read_obj1;
		return;
	}

	val[0] = read_buffer[0] | (((uint16_t)read_buffer[1]) << 8);

	MLX90614_read2ByteValue(addr, (uint8_t)OBJ1, rs);
	enum status_code read_obj2 =  rs.return_status;

	if ((read_obj2 & 0x0F) != 0) {
		rs.return_status = read_obj2;
		return;
	}

	val[1] = read_buffer[0] | (((uint16_t)read_buffer[1]) << 8);

	rs.return_status = read_obj2;
	rs.return_value = (val[0] + val[1])/2;

}

// converts a data value from the sensor corresponding to a temperature memory address to a Celsius temperature
float dataToTemp(uint16_t data){
	float raw_temp = (float)data; // treat data as a 16-bit float
	return 0.02*raw_temp - 273.15;
}

// Read target temperature in degrees Celsius
// temp_target can be AMBIENT, OBJ1, OBJ2
void MLX90614_readTempC(MLXDeviceAddr addr, IRTempTarget temp_target, return_struct_float returner) {
	return_struct_16 rs;
	MLX90614_read2ByteValue(addr, (uint8_t) temp_target, rs);
	returner.return_status = rs.return_status;
	returner.return_value = dataToTemp(rs.return_value);
}

// sanity check, should always return the same value as device_addr
void MLX90614_getAddress(MLXDeviceAddr addr, return_struct_16 rs) {
	MLX90614_read2ByteValue(addr, MLX90614_SMBUS, rs);
}

// set the sensor to SLEEP mode (BROKEN)
enum status_code MLX90614_setSleepMode(MLXDeviceAddr addr) {
	// enter SLEEP mode command = 1111_1111 (0xFF)
	// send PEC (Packet Error Code) (0xF3)
	uint8_t dataByte[2] = {(uint8_t)0xFF, (uint8_t)0xF3};
	// write to address with stop condition
	enum status_code statc_sleep = writeDataToAddress(dataByte, 2, addr, true);

}
