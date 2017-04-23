#include "MPU9250_9axis_Commands.h"

enum status_code MPU9250_gyro_init(void) {
	uint8_t gyroData[] = {GYRO_CONFIG_ADDRESS,GYRO_FULL_SCALE_2000_DPS};
	return writeDataToAddress(gyroData,2,MPU9250_ADDRESS,MPU9250_SHOULD_STOP);
}

enum status_code MPU9250_acc_init() {
	uint8_t accData[] = {ACC_CONFIG_ADDRESS,ACC_FULL_SCALE_16_G};
	return writeDataToAddress(accData,2,MPU9250_ADDRESS,MPU9250_SHOULD_STOP);
}

enum status_code MPU9250_mag_init() {
	uint8_t magData[] = {MAG_PASSTHROUGH_ADDRESS,MAG_PASSTHROUGH_MODE};
	return writeDataToAddress(magData,2,MPU9250_ADDRESS,MPU9250_SHOULD_STOP);
}

enum status_code MPU9250_read_mag(MPU9250Reading* toFill){
	// Request single magnetometer read to be performed
	uint8_t reqData[] = {MAG_REQUEST_ADDRESS,MAG_SINGLE_MEASUREMENT};
	enum status_code statc1 = writeDataToAddress(reqData,2,MAG_ADDRESS,MPU9250_SHOULD_STOP);
	
	// Check if measurement is ready
	uint8_t status[1] = {!MAG_DATA_READY};
	while(!(status[0]&MAG_DATA_READY)){
		readFromAddressAndMemoryLocation(status,1,MAG_ADDRESS,MAG_STATUS_ADDRESS,MPU9250_SHOULD_STOP);
	}
	
	//Read data
	uint8_t data[6] = {0,0,0,0,0,0};
	enum status_code statc2 = readFromAddressAndMemoryLocation(data,6,MAG_ADDRESS,MAG_READ_ADDRESS,MPU9250_SHOULD_STOP);
	
	//process data
	toFill->mag.x = -(data[3]<<8 | data[2]);
	toFill->mag.y = -(data[1]<<8 | data[0]);
	toFill->mag.z = -(data[5]<<8 | data[4]);
	
	if (statc1 & 0xf0 != 0) {
		return statc1;
	} else {
		return statc2;
	}
}

enum status_code MPU9250_read_acc(MPU9250Reading* toFill){	
	uint8_t data[6] = {0,0,0,0,0,0};
	//Read data
	enum status_code statc = readFromAddressAndMemoryLocation(data,6,MPU9250_ADDRESS,ACC_READ_ADDRESS,MPU9250_SHOULD_STOP);
	
	//process data
	toFill->accel.x=-(data[0]<<8 | data[1]);
	toFill->accel.y=-(data[2]<<8 | data[3]);
	toFill->accel.z=data[4]<<8 | data[5];
	
	return statc;
}

enum status_code MPU9250_read_gyro(MPU9250Reading* toFill){	
	uint8_t data[6] = {0,0,0,0,0,0};
	//Read data
	enum status_code statc = readFromAddressAndMemoryLocation(data,6,MPU9250_ADDRESS,GYRO_READ_ADDRESS,MPU9250_SHOULD_STOP);
	
	//process data
	toFill->gyro.x=-(data[0]<<8 | data[1]);
	toFill->gyro.y=-(data[2]<<8 | data[3]);
	toFill->gyro.z=data[4]<<8 | data[5];
	
	return statc;
}

void initReading(MPU9250Reading* toFill){
	toFill->accel.x=0;
	toFill->accel.y=0;
	toFill->accel.z=0;
	
	toFill->gyro.x=0;
	toFill->gyro.y=0;
	toFill->gyro.z=0;
	
	toFill->mag.x=0;
	toFill->mag.y=0;
	toFill->mag.z=0;
}

typedef struct {
	float x, y, z;
} HumanAxisReading;

typedef struct {
	HumanAxisReading mag, accel, gyro;
} HumanReading;

//accelerometer in g
//gyro in degrees/sec
//magnetometer in microteslas
HumanReading humanReadableOutput(MPU9250Reading input){
	HumanReading output;
	
	float rangeMax = 32767.0;
	
	output.accel.x=(input.accel.x/rangeMax)*16;
	output.accel.y=(input.accel.y/rangeMax)*16;
	output.accel.z=(input.accel.z/rangeMax)*16;
	
	output.gyro.x=(input.gyro.x/rangeMax)*2000;
	output.gyro.y=(input.gyro.y/rangeMax)*2000;
	output.gyro.z=(input.gyro.z/rangeMax)*2000;
	
	output.mag.x=(input.mag.x/rangeMax)*4800;
	output.mag.y=(input.mag.y/rangeMax)*4800;
	output.mag.z=(input.mag.z/rangeMax)*4800;
	
	return output;
}

MPU9250Reading MPU9250_read(void){
	MPU9250Reading output;
	
	initReading(&output);
	
	enum status_code statc1 = MPU9250_read_acc(&output);
	enum status_code statc2 = MPU9250_read_gyro(&output);
	//MPU9250_read_mag(&output);
	
	HumanReading reading = humanReadableOutput(output);
	
	float direction = MPU9250_computeCompassDir(reading.mag.x, reading.mag.y, reading.mag.z);
	
	return output;
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