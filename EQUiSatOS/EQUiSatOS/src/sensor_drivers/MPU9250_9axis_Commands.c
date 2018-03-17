#include "MPU9250_9axis_Commands.h"

enum status_code MPU9250_init(void) {
	uint8_t address = 0;
	return readFromAddressAndMemoryLocation(&address,1,MPU9250_ADDRESS,WHOAMI_ADDRESS,MPU9250_SHOULD_STOP);
}

enum status_code gyro_init(void) {
	uint8_t gyroData[] = {GYRO_CONFIG_ADDRESS,GYRO_FULL_SCALE_250_DPS};
	return writeDataToAddress(gyroData,2,MPU9250_ADDRESS,MPU9250_SHOULD_STOP);
}

enum status_code accel_init(void) {
	uint8_t accData[] = {ACC_CONFIG_ADDRESS,ACC_FULL_SCALE_2_G};
	return writeDataToAddress(accData,2,MPU9250_ADDRESS,MPU9250_SHOULD_STOP);
}

enum status_code mag_init(void) {
	uint8_t magData[] = {MAG_PASSTHROUGH_ADDRESS,MAG_PASSTHROUGH_MODE};
	return writeDataToAddress(magData,2,MPU9250_ADDRESS,MPU9250_SHOULD_STOP);
}

void convert_MPU9250_to_EQUiSat_coords(int16_t toFill[3]) {
	toFill[0] *= -1;
	int16_t temp = toFill[2];
	toFill[2] = toFill[1]*-1;
	toFill[1] = temp;
}

enum status_code MPU9250_read_mag(int16_t toFill[3]){
	// Request single magnetometer read to be performed
	uint8_t reqData[] = {MAG_REQUEST_ADDRESS,MAG_SINGLE_MEASUREMENT};
	//enum status_code statc1 = 
	writeDataToAddress(reqData,2,MAG_ADDRESS,MPU9250_SHOULD_STOP);
	
	// Check if measurement is ready
	uint8_t status[1] = {!MAG_DATA_READY};
	while(!(status[0]&MAG_DATA_READY)){
		readFromAddressAndMemoryLocation(status,1,MAG_ADDRESS,MAG_STATUS_ADDRESS,MPU9250_SHOULD_STOP);
	}
	
	//Read data
	uint8_t data[6] = {0,0,0,0,0,0};
	enum status_code statc2 = 
	readFromAddressAndMemoryLocation(data,6,MAG_ADDRESS,MAG_READ_ADDRESS,MPU9250_SHOULD_STOP);
	
	//process data
	toFill[0] = -(data[3]<<8 | data[2]);
	toFill[1] = -(data[1]<<8 | data[0]);
	toFill[2] = -(data[5]<<8 | data[4]);
	
	return statc2;
}

enum status_code MPU9250_read_mag_EQUiSat_coords(int16_t toFill[3]) {
	enum status_code status = MPU9250_read_mag(toFill);
	convert_MPU9250_to_EQUiSat_coords(toFill);
	return status;
}

//toFill in the order of xyz
enum status_code MPU9250_read_acc(int16_t toFill[3]){
	uint8_t data[6] = {0,0,0,0,0,0};
	//Read data
	enum status_code statc = readFromAddressAndMemoryLocation(data,6,MPU9250_ADDRESS,ACC_READ_ADDRESS,MPU9250_SHOULD_STOP);
	
	//process data
	toFill[0]=-(data[0]<<8 | data[1]);
	toFill[1]=-(data[2]<<8 | data[3]);
	toFill[2]=data[4]<<8 | data[5];
	
	return statc;
}

enum status_code MPU9250_read_acc_EQUiSat_coords(int16_t toFill[3]) {	
	enum status_code status = MPU9250_read_acc(toFill);
	convert_MPU9250_to_EQUiSat_coords(toFill);
	return status;
}


enum status_code MPU9250_read_gyro(int16_t toFill[3]){	
	uint8_t data[6] = {0,0,0,0,0,0};
	//Read data
	enum status_code statc = readFromAddressAndMemoryLocation(data,6,MPU9250_ADDRESS,GYRO_READ_ADDRESS,MPU9250_SHOULD_STOP);
	
	//process data
	toFill[0]=-(data[0]<<8 | data[1]);
	toFill[1]=-(data[2]<<8 | data[3]);
	toFill[2]=data[4]<<8 | data[5];
	
	return statc;
}

enum status_code MPU9250_read_gyro_EQUiSat_coords(int16_t toFill[3]) {
	enum status_code status = MPU9250_read_gyro(toFill);
	convert_MPU9250_to_EQUiSat_coords(toFill);
	return status;
}

// to convert to Celsius: buf / 333.87 + 21.0
enum status_code MPU9250_read_temp(int16_t* buf) {
	uint8_t rawData[2];  // each byte of the data
	enum status_code statc = readFromAddressAndMemoryLocation(rawData,2,MPU9250_ADDRESS,TEMP_READ_ADDRESS,MPU9250_SHOULD_STOP); 
	*buf = ((int16_t)rawData[0] << 8) | rawData[1];  // Turn the MSB and LSB into a 16-bit value
	return statc;
}

float MPU9250_computeCompassDir(int16_t x, int16_t y, int16_t z) {
	float heading = atan2(z, x);
	float declinationAngle = 0.244346;
	heading += declinationAngle;
	// Correct for when signs are reversed.
	if(heading < 0) {
		heading += 2*M_PI;	
	}	

	// Check for wrap due to addition of declination.
	if(heading > 2*M_PI) {
		heading -= 2*M_PI;	
	}	
	float headingDegrees = heading * 180/M_PI;	
	return headingDegrees;
}

//Compute the biases from averaging surrounding inputs and loads them to bias registers i.e. "zeros" the IMU.
// Should be run with the sensor flat and not moving.
void MPU9250_computeBias(float* dest1, float* dest2){
	uint8_t data[12]; // data array to hold accelerometer and gyro x, y, z, data
	uint16_t ii, packet_count, fifo_count;
	uint32_t gyro_bias[3]  = {0, 0, 0}, accel_bias[3] = {0, 0, 0};
	
	uint8_t dataByte[1], subaddress[1];
	
	delay_init();
	
	// reset device
	dataByte[0] = (uint8_t)0x80; subaddress[0] = PWR_MGMT_1;
	//enum status_code statc1 =
	writeDataToAddressSub(dataByte,2,MPU9250_ADDRESS,subaddress,MPU9250_SHOULD_STOP);
	delay_ms(100);
	
	// get stable time source; Auto select clock source to be PLL gyroscope reference if ready
	// else use the internal oscillator, bits 2:0 = 001
	dataByte[0] = (uint8_t)0x01;
	//enum status_code statc2 = 
	writeDataToAddressSub(dataByte,2,MPU9250_ADDRESS,subaddress,MPU9250_SHOULD_STOP);
	
	dataByte[0] = (uint8_t)0x00; subaddress[0] = PWR_MGMT_2;
	//enum status_code statc3 = 
	writeDataToAddressSub(dataByte,2,MPU9250_ADDRESS,subaddress,MPU9250_SHOULD_STOP);
	delay_ms(200);

	// Configure device for bias calculation
	subaddress[0] = INT_ENABLE;
	//enum status_code statc4 =
	writeDataToAddressSub(dataByte,2,MPU9250_ADDRESS,subaddress,MPU9250_SHOULD_STOP); //disable all interrupts
	
	subaddress[0] = FIFO_EN;
	//enum status_code statc5 =
	writeDataToAddressSub(dataByte,2,MPU9250_ADDRESS,subaddress,MPU9250_SHOULD_STOP); //disable FIFO
	
	subaddress[0] = PWR_MGMT_1;
	//enum status_code statc6 =
	writeDataToAddressSub(dataByte,2,MPU9250_ADDRESS,subaddress,MPU9250_SHOULD_STOP); //turn on internal clock source
	
	subaddress[0] = I2C_MST_CTRL;
	//enum status_code statc7 =
	writeDataToAddressSub(dataByte,2,MPU9250_ADDRESS,subaddress,MPU9250_SHOULD_STOP); //disable I2C master
	
	subaddress[0] = USER_CTRL;
	//enum status_code statc8 =
	writeDataToAddressSub(dataByte,2,MPU9250_ADDRESS,subaddress,MPU9250_SHOULD_STOP); //disable FIFO and I2C master
	
	dataByte[0] = (uint8_t)0x0C;
	//enum status_code statc9 =
	writeDataToAddressSub(dataByte,2,MPU9250_ADDRESS,subaddress,MPU9250_SHOULD_STOP); //reset FIFO and DMP
	
	delay_ms(15);
	
	// Configure MPU6050 gyro and accelerometer for bias calculation
	dataByte[0] = (uint8_t)0x01; subaddress[0] = CONFIG;
	//enum status_code statc10 =
	writeDataToAddressSub(dataByte,2,MPU9250_ADDRESS,subaddress,MPU9250_SHOULD_STOP); //set lpf to 188hz
	
	dataByte[0] = (uint8_t)0x00; subaddress[0] = SMPLRT_DIV;
	//enum status_code statc11 =
	writeDataToAddressSub(dataByte,2,MPU9250_ADDRESS,subaddress,MPU9250_SHOULD_STOP); //set sample rate to 1 khz
	
	subaddress[0] = GYRO_CONFIG_ADDRESS;
	//enum status_code statc12 =
	writeDataToAddressSub(dataByte,2,MPU9250_ADDRESS,subaddress,MPU9250_SHOULD_STOP); // Set gyro full-scale to 250 degrees per second
	
	subaddress[0] = ACC_CONFIG_ADDRESS;
	//enum status_code statc13 =
	writeDataToAddressSub(dataByte,2,MPU9250_ADDRESS,subaddress,MPU9250_SHOULD_STOP); // Set accelerometer full-scale to 2 g, maximum sensitivity
	
	//uhhhh not really sure how we got these but oh well
	uint16_t  gyrosensitivity  = 131;   // = 131 LSB/degrees/sec
	uint16_t  accelsensitivity = 16384;  // = 16384 LSB/g

	// Configure FIFO to capture accelerometer and gyro data for bias calculation
	dataByte[0] = (uint8_t)0x40; subaddress[0] = USER_CTRL;
	//enum status_code statc14 =
	writeDataToAddressSub(dataByte,2,MPU9250_ADDRESS,subaddress,MPU9250_SHOULD_STOP); // Enable FIFO
	
	dataByte[0] = (uint8_t)0x78; subaddress[0] = FIFO_EN;
	//enum status_code statc15 =
	writeDataToAddressSub(dataByte,2,MPU9250_ADDRESS,subaddress,MPU9250_SHOULD_STOP); // Enable gyro and accelerometer sensors for FIFO  (max size 512 bytes in MPU-9150)
	delay_ms(40); // accumulate 40 samples in 40 milliseconds = 480 bytes

	// At end of sample accumulation, turn off FIFO sensor read
	dataByte[0] = (uint8_t)0x00;
	//enum status_code statc16 =
	writeDataToAddressSub(dataByte,2,MPU9250_ADDRESS,subaddress,MPU9250_SHOULD_STOP); // Disable gyro and accelerometer sensors for FIFO
	
	//enum status_code statc17 =
	readFromAddressAndMemoryLocation(&data[0],2,MPU9250_ADDRESS,FIFO_COUNTH,MPU9250_SHOULD_STOP);
	fifo_count = ((uint16_t)data[0] << 8) | data[1];
	packet_count = fifo_count/12;// How many sets of full gyro and accelerometer data for averaging
	
	for (ii = 0; ii < packet_count; ii++) {
		uint16_t accel_temp[3] = {0, 0, 0}, gyro_temp[3] = {0, 0, 0};
		//enum status_code statc18 =
		readFromAddressAndMemoryLocation(&data[0],12,MPU9250_ADDRESS,FIFO_R_W,MPU9250_SHOULD_STOP); //read data to average
		accel_temp[0] = (int16_t) (((int16_t)data[0] << 8) | data[1]  ) ;  // Form signed 16-bit integer for each sample in FIFO
		accel_temp[1] = (int16_t) (((int16_t)data[2] << 8) | data[3]  ) ;
		accel_temp[2] = (int16_t) (((int16_t)data[4] << 8) | data[5]  ) ;
		gyro_temp[0]  = (int16_t) (((int16_t)data[6] << 8) | data[7]  ) ;
		gyro_temp[1]  = (int16_t) (((int16_t)data[8] << 8) | data[9]  ) ;
		gyro_temp[2]  = (int16_t) (((int16_t)data[10] << 8) | data[11]) ;
		
		accel_bias[0] += (int32_t) accel_temp[0]; // Sum individual signed 16-bit biases to get accumulated signed 32-bit biases
		accel_bias[1] += (int32_t) accel_temp[1];
		accel_bias[2] += (int32_t) accel_temp[2];
		gyro_bias[0]  += (int32_t) gyro_temp[0];
		gyro_bias[1]  += (int32_t) gyro_temp[1];
		gyro_bias[2]  += (int32_t) gyro_temp[2];
		
	}
	accel_bias[0] /= (int32_t) packet_count; // Normalize sums to get average count biases
	accel_bias[1] /= (int32_t) packet_count;
	accel_bias[2] /= (int32_t) packet_count;
	gyro_bias[0]  /= (int32_t) packet_count;
	gyro_bias[1]  /= (int32_t) packet_count;
	gyro_bias[2]  /= (int32_t) packet_count;
	
	if(accel_bias[2] > 0L) {accel_bias[2] -= (int32_t) accelsensitivity;}  // Remove gravity from the z-axis accelerometer bias calculation
	else {accel_bias[2] += (int32_t) accelsensitivity;}
	
	// Construct the gyro biases for push to the hardware gyro bias registers, which are reset to zero upon device startup
	data[0] = (-gyro_bias[0]/4  >> 8) & 0xFF; // Divide by 4 to get 32.9 LSB per deg/s to conform to expected bias input format
	data[1] = (-gyro_bias[0]/4)       & 0xFF; // Biases are additive, so change sign on calculated average gyro biases
	data[2] = (-gyro_bias[1]/4  >> 8) & 0xFF;
	data[3] = (-gyro_bias[1]/4)       & 0xFF;
	data[4] = (-gyro_bias[2]/4  >> 8) & 0xFF;
	data[5] = (-gyro_bias[2]/4)       & 0xFF;
	
	// Push gyro biases to hardware registers
	subaddress[0] = XG_OFFSET_H;
	//enum status_code statc19 =
	writeDataToAddressSub(&data[0],2,MPU9250_ADDRESS,subaddress,MPU9250_SHOULD_STOP);
	
	subaddress[0] = XG_OFFSET_L;
	//enum status_code statc20 =
	writeDataToAddressSub(&data[1],2,MPU9250_ADDRESS,subaddress,MPU9250_SHOULD_STOP);
	
	subaddress[0] = YG_OFFSET_H;
	//enum status_code statc21 =
	writeDataToAddressSub(&data[2],2,MPU9250_ADDRESS,subaddress,MPU9250_SHOULD_STOP);
	
	subaddress[0] = YG_OFFSET_L;
	//enum status_code statc22 =
	writeDataToAddressSub(&data[3],2,MPU9250_ADDRESS,subaddress,MPU9250_SHOULD_STOP);
	
	subaddress[0] = ZG_OFFSET_H;
	//enum status_code statc23 =
	writeDataToAddressSub(&data[4],2,MPU9250_ADDRESS,subaddress,MPU9250_SHOULD_STOP);
	
	subaddress[0] = ZG_OFFSET_L;
	//enum status_code statc24 =
	writeDataToAddressSub(&data[5],2,MPU9250_ADDRESS,subaddress,MPU9250_SHOULD_STOP);
	
	
	// Output scaled gyro biases for display in the main program
	dest1[0] = (float) gyro_bias[0]/gyrosensitivity;
	dest1[1] = (float) gyro_bias[1]/gyrosensitivity;
	dest1[2] = (float) gyro_bias[2]/gyrosensitivity;

	// Construct the accelerometer biases and push to the hardware accelerometer bias registers. These registers contain
	// factory trim values which must be added to the calculated accelerometer biases; on boot up these registers will hold
	// non-zero values. In addition, bit 0 of the lower byte must be preserved since it is used for temperature
	// compensation calculations. Accelerometer bias registers expect bias input as 2048 LSB per g, so that
	// the accelerometer biases calculated above must be divided by 8.

	int32_t accel_bias_reg[3] = {0, 0, 0}; // A place to hold the factory accelerometer trim biases
	//enum status_code statc25 =
	readFromAddressAndMemoryLocation(&data[0],2,MPU9250_ADDRESS,XA_OFFSET_H,MPU9250_SHOULD_STOP); // Read factory accelerometer trim values
	accel_bias_reg[0] = (int32_t) (((int16_t)data[0] << 8) | data[1]);
	
	//enum status_code statc26 =
	readFromAddressAndMemoryLocation(&data[0],2,MPU9250_ADDRESS,YA_OFFSET_H,MPU9250_SHOULD_STOP);
	accel_bias_reg[1] = (int32_t) (((int16_t)data[0] << 8) | data[1]);
	
	//enum status_code statc27 =
	readFromAddressAndMemoryLocation(&data[0],2,MPU9250_ADDRESS,ZA_OFFSET_H,MPU9250_SHOULD_STOP);
	accel_bias_reg[2] = (int32_t) (((int16_t)data[0] << 8) | data[1]);
	
	uint32_t mask = 1uL; // Define mask for temperature compensation bit 0 of lower byte of accelerometer bias registers
	uint8_t mask_bit[3] = {0, 0, 0}; // Define array to hold mask bit for each accelerometer bias axis
	
	for(ii = 0; ii < 3; ii++) {
		if((accel_bias_reg[ii] & mask)) mask_bit[ii] = 0x01; // If temperature compensation bit is set, record that fact in mask_bit
	}
	
	// Construct total accelerometer bias, including calculated average accelerometer bias from above
	accel_bias_reg[0] -= (accel_bias[0]/8); // Subtract calculated averaged accelerometer bias scaled to 2048 LSB/g (16 g full scale)
	accel_bias_reg[1] -= (accel_bias[1]/8);
	accel_bias_reg[2] -= (accel_bias[2]/8);
	
	data[0] = (accel_bias_reg[0] >> 8) & 0xFF;
	data[1] = (accel_bias_reg[0])      & 0xFF;
	data[1] = data[1] | mask_bit[0]; // preserve temperature compensation bit when writing back to accelerometer bias registers
	data[2] = (accel_bias_reg[1] >> 8) & 0xFF;
	data[3] = (accel_bias_reg[1])      & 0xFF;
	data[3] = data[3] | mask_bit[1]; // preserve temperature compensation bit when writing back to accelerometer bias registers
	data[4] = (accel_bias_reg[2] >> 8) & 0xFF;
	data[5] = (accel_bias_reg[2])      & 0xFF;
	data[5] = data[5] | mask_bit[2]; // preserve temperature compensation bit when writing back to accelerometer bias registers
	
	// According to the arduino library this code was based on the push to acceleration biases in the MPU-9250
	// We get good statuses from all our writes so let's just assume it works. If it doesn't we probably aren't handling
	// the temperature bit correctly


	// Push accelerometer biases to hardware registers
	uint8_t check[2];
	
	subaddress[0] = XA_OFFSET_H;
	//enum status_code statc28 =
	writeDataToAddressSub(&data[0],2,MPU9250_ADDRESS,subaddress,MPU9250_SHOULD_STOP);
	
	//enum status_code stat1 = 
	readFromAddressAndMemoryLocation(&check[0],2,MPU9250_ADDRESS,XA_OFFSET_H,MPU9250_SHOULD_STOP);
	
	subaddress[0] = XA_OFFSET_L_TC;
	//enum status_code statc29 =
	writeDataToAddressSub(&data[1],2,MPU9250_ADDRESS,subaddress,MPU9250_SHOULD_STOP);
	
	//enum status_code stat2 = 
	readFromAddressAndMemoryLocation(&check[0],2,MPU9250_ADDRESS,XA_OFFSET_L_TC,MPU9250_SHOULD_STOP);
	
	subaddress[0] = YA_OFFSET_H;
	//enum status_code statc30 =
	writeDataToAddressSub(&data[2],2,MPU9250_ADDRESS,subaddress,MPU9250_SHOULD_STOP);
	
	//enum status_code stat3 = 
	readFromAddressAndMemoryLocation(&check[0],2,MPU9250_ADDRESS,YA_OFFSET_H,MPU9250_SHOULD_STOP);
	
	subaddress[0] = YA_OFFSET_L_TC;
	//enum status_code statc31 =
	writeDataToAddressSub(&data[3],2,MPU9250_ADDRESS,subaddress,MPU9250_SHOULD_STOP);
	
	//enum status_code stat4 = 
	readFromAddressAndMemoryLocation(&check[0],2,MPU9250_ADDRESS,YA_OFFSET_L_TC,MPU9250_SHOULD_STOP);
	
	subaddress[0] = ZA_OFFSET_H;
	//enum status_code statc32 =
	writeDataToAddressSub(&data[4],2,MPU9250_ADDRESS,subaddress,MPU9250_SHOULD_STOP);
	
	//enum status_code stat5 = 
	readFromAddressAndMemoryLocation(&check[0],2,MPU9250_ADDRESS,ZA_OFFSET_H,MPU9250_SHOULD_STOP);
	
	subaddress[0] = ZA_OFFSET_L_TC;
	//enum status_code statc33 =
	writeDataToAddressSub(&data[5],2,MPU9250_ADDRESS,subaddress,MPU9250_SHOULD_STOP);
	
	//enum status_code stat6 = 
	readFromAddressAndMemoryLocation(&check[0],2,MPU9250_ADDRESS,ZA_OFFSET_L_TC,MPU9250_SHOULD_STOP);
	// Output scaled accelerometer biases for display in the main program
	dest2[0] = (float) accel_bias[0]/accelsensitivity;
	dest2[1] = (float) accel_bias[1]/accelsensitivity;
	dest2[2] = (float) accel_bias[2]/accelsensitivity;
}