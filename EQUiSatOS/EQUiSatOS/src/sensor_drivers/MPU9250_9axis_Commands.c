#include "MPU9250_9axis_Commands.h"

void MPU9250_init(void) {
	uint8_t address = 0;
	
	readFromAddressAndMemoryLocation(&address,1,MPU9250_ADDRESS,WHOAMI_ADDRESS,MPU9250_SHOULD_STOP);
	
	uint8_t gyroData[] = {GYRO_CONFIG_ADDRESS,GYRO_FULL_SCALE_2000_DPS};
	uint8_t accData[] = {ACC_CONFIG_ADDRESS,ACC_FULL_SCALE_16_G};
	uint8_t magData[] = {MAG_PASSTHROUGH_ADDRESS,MAG_PASSTHROUGH_MODE};
	
	writeDataToAddress(gyroData,2,MPU9250_ADDRESS,MPU9250_SHOULD_STOP);
	writeDataToAddress(accData,2,MPU9250_ADDRESS,MPU9250_SHOULD_STOP);
	writeDataToAddress(magData,2,MPU9250_ADDRESS,MPU9250_SHOULD_STOP);
}

void MPU9250_read_mag(MPU9250Reading* toFill){
	// Request single magnetometer read to be performed
	uint8_t reqData[] = {MAG_REQUEST_ADDRESS,MAG_SINGLE_MEASUREMENT};
	writeDataToAddress(reqData,2,MAG_ADDRESS,MPU9250_SHOULD_STOP);
	
	// Check if measurement is ready
	uint8_t status[1] = {!MAG_DATA_READY};
	while(!(status[0]&MAG_DATA_READY)){
		readFromAddressAndMemoryLocation(status,1,MAG_ADDRESS,MAG_STATUS_ADDRESS,MPU9250_SHOULD_STOP);
	}
	
	//Read data
	uint8_t data[6] = {0,0,0,0,0,0};
	readFromAddressAndMemoryLocation(data,6,MAG_ADDRESS,MAG_READ_ADDRESS,MPU9250_SHOULD_STOP);
	
	//process data
	toFill->mag.x = -(data[3]<<8 | data[2]);
	toFill->mag.y = -(data[1]<<8 | data[0]);
	toFill->mag.z = -(data[5]<<8 | data[4]);
}

void MPU9250_read_acc(MPU9250Reading* toFill){	
	uint8_t data[6] = {0,0,0,0,0,0};
	//Read data
	readFromAddressAndMemoryLocation(data,6,MPU9250_ADDRESS,ACC_READ_ADDRESS,MPU9250_SHOULD_STOP);
	
	//process data
	toFill->accel.x=-(data[0]<<8 | data[1]);
	toFill->accel.y=-(data[2]<<8 | data[3]);
	toFill->accel.z=data[4]<<8 | data[5];
}

void MPU9250_read_gyro(MPU9250Reading* toFill){	
	uint8_t data[6] = {0,0,0,0,0,0};
	//Read data
	readFromAddressAndMemoryLocation(data,6,MPU9250_ADDRESS,GYRO_READ_ADDRESS,MPU9250_SHOULD_STOP);
	
	//process data
	toFill->gyro.x=-(data[0]<<8 | data[1]);
	toFill->gyro.y=-(data[2]<<8 | data[3]);
	toFill->gyro.z=data[4]<<8 | data[5];
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
	
	MPU9250_read_acc(&output);
	MPU9250_read_gyro(&output);
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