#ifndef _MPU9250_9AXIS_COMMANDS_H_
#define _MPU9250_9AXIS_COMMANDS_H_

#include "../processor_drivers/I2C_Commands.h"

#define		MPU9250_ADDRESS				0x68
#define		MAG_ADDRESS					0x0C

//ram addresses (MPU)
#define		GYRO_CONFIG_ADDRESS			0x1B
#define		ACC_CONFIG_ADDRESS			0x1C
#define		MAG_PASSTHROUGH_ADDRESS		0x37
#define		ACC_READ_ADDRESS			0x3B
#define		GYRO_READ_ADDRESS			0x43
#define		WHOAMI_ADDRESS				0x75

//ram addresses (Magnetometer)
#define		MAG_REQUEST_ADDRESS			0x0A
#define		MAG_STATUS_ADDRESS			0x02
#define		MAG_READ_ADDRESS			0x03

//configuration parameters
#define		MAG_PASSTHROUGH_MODE		0x02

#define		GYRO_FULL_SCALE_250_DPS		0x00
#define		GYRO_FULL_SCALE_500_DPS		0x08
#define		GYRO_FULL_SCALE_1000_DPS	0x10
#define		GYRO_FULL_SCALE_2000_DPS	0x18

#define		ACC_FULL_SCALE_2_G			0x00
#define		ACC_FULL_SCALE_4_G			0x08
#define		ACC_FULL_SCALE_8_G			0x10
#define		ACC_FULL_SCALE_16_G			0x18

#define		MAG_SINGLE_MEASUREMENT		0x01

#define		MAG_DATA_READY				0x01

i2c_func MPU9250_i2c_write_func;
i2c_func MPU9250_i2c_read_func;
i2c_func MPU9250_i2c_write_no_stop_func;

typedef struct {
	int16_t x, y, z;
} ThreeAxisReading;

typedef struct {
	ThreeAxisReading mag, accel, gyro;
} MPU9250Reading;

void writeDataToAddress(uint8_t* data, uint8_t len, uint8_t address);
void readFromAddressAndMemoryLocation(uint8_t* buffer, uint8_t len, uint8_t address, uint8_t memoryLocation);
void MPU9250_init(i2c_func _i2c_write_func, i2c_func _i2c_read_func, i2c_func _i2c_write_no_stop_func);
void MPU9250_read_mag(MPU9250Reading* toFill);
void MPU9250_read_acc(MPU9250Reading* toFill);
void MPU9250_read_gyro(MPU9250Reading* toFill);
MPU9250Reading MPU9250_read();


#endif