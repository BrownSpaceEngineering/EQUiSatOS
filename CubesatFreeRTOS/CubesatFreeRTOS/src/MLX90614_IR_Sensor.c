#include <MLX90614_IR_Sensor.h>

/* Read 2 bytes over I2C from an MLX device.
    device_addr: I2C address of the device
    mem_addr: RAM or EEPROM address to read OR'd with its opcode (see datasheet)
    buf: 2-byte destination data buffer
*/
void read_MLX90614(uint8_t device_addr, uint8_t mem_addr, uint8_t* buf) {
	
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
	
	i2c_write_command_no_stop(&write_packet);
	i2c_read_command(&read_packet);
}

/* Write a value to an MLX's EEPROM e.g. to modify its SMBus address.
    device_addr: I2C address of the device
    mem_addr: EEPROM address to write to OR'd with its opcode (see datasheet)
    buf: 2-byte source data buffer
*/
void write_MLX90614_eeprom(uint8_t device_addr, uint8_t mem_addr, uint8_t* buf) {
	
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
	
	i2c_write_command(&write_packet);
}

//reads a 2 byte value which is sorted:
//if lsb_first=true, lsb is the 0 entry
//if lsb_first=false, lsb is the 1 entry
uint16_t MLX90614_read2ByteValue(uint16_t device_addr, uint8_t mem_addr, bool lsb_first){
	uint8_t read_buffer[2] = {
		0x0,0x0
	};
	read_MLX90614(device_addr, mem_addr,read_buffer);
	uint16_t res = 0;
	if(lsb_first){
		res = read_buffer[0] | (((uint16_t)read_buffer[1]) << 8);
	}else{
		res = read_buffer[1] | (((uint16_t)read_buffer[0]) << 8);
	}
	return res;
}

//Function to read raw ir data from sensor.  is_ir2 is a boolean to determine which sensor to read from, 1 if false, 2 if true
uint16_t MLX90614_readRawIRData(uint8_t device_addr, bool is_ir2)
{
	uint8_t data_addr = 0;
	if(is_ir2){
		data_addr = MLX90614_RAWIR2;
	}else{
		data_addr = MLX90614_RAWIR1;
	}
	
	//unknown what the structure of the rawir data is, LSB MSB or MSB LSB. Assumed LSB MSB like temp data but unsure
	return MLX90614_read2ByteValue(device_addr,data_addr,true);
}

//converts a data value from the sensor corresponding to a temperature memory address to a celsius temperature
float dataToTemp(uint16_t data){
	float temp = (float) data;
	temp *= .02;
	temp -= 273.15;
	return temp;
}

//reads object temp if is_ambient is false, ambient if true
float MLX90614_readTempC(uint8_t device_addr, bool is_ambient) {
	uint8_t data_addr = 0;
	
	if(is_ambient){
		data_addr = MLX90614_TA;
	}else{
		data_addr = MLX90614_TOBJ1;
	}
	
	uint16_t data = MLX90614_read2ByteValue(device_addr,data_addr,true);
	float temp = dataToTemp(data);
	return temp;
}

//recursive check, should always return the same value as device_addr
uint16_t MLX90614_getAddress(uint8_t device_addr){
	uint16_t mem = MLX90614_read2ByteValue(device_addr,MLX90614_SMBUS,false);
	return mem;
}

//changes device address from current_addr to new_addr. make sure new_addr doesnt conflict with any networked i2c devices
void MLX90614_setAddress(uint8_t current_addr, uint8_t new_addr){
	uint16_t mem = MLX90614_read2ByteValue(current_addr,MLX90614_SMBUS,false);
	uint8_t* raws = (uint8_t*) &mem;
	raws[1] = new_addr;
	write_MLX90614_eeprom(current_addr,MLX90614_SMBUS,raws);
}

//http://www.barrgroup.com/Embedded-Systems/How-To/CRC-Calculation-C-Code
uint8_t crc(uint8_t* message, int nBytes)
{	
    uint8_t remainder = 0;	
    /*
     * Perform modulo-2 division, a byte at a time.
     */
    for (int byte = 0; byte < nBytes; ++byte)
    {
        /*
         * Bring the next byte into the remainder.
         */
        remainder ^= (message[byte] << (WIDTH - 8));

        /*
         * Perform modulo-2 division, a bit at a time.
         */
        for (uint8_t bit = 8; bit > 0; --bit)
        {
            /*
             * Try to divide the current data bit.
             */
            if (remainder & TOPBIT)
            {
                remainder = (remainder << 1) ^ CRC_POLYNOMIAL;
            }
            else
            {
                remainder = (remainder << 1);
            }
        }
    }

    /*
     * The final remainder is the CRC result.
     */
    return (remainder);
}

