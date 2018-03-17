#ifndef _MPU9250_9AXIS_COMMANDS_H_
#define _MPU9250_9AXIS_COMMANDS_H_

#include "../processor_drivers/I2C_Commands.h"
#include <math.h>

#define		MPU9250_SHOULD_STOP			true

#define		MPU9250_ADDRESS				0x68
#define		MAG_ADDRESS					0x0C

//ram addresses (MPU)
#define		GYRO_CONFIG_ADDRESS			0x1B
#define		ACC_CONFIG_ADDRESS			0x1C
#define		MAG_PASSTHROUGH_ADDRESS		0x37
#define		ACC_READ_ADDRESS			0x3B
#define		GYRO_READ_ADDRESS			0x43
#define		TEMP_READ_ADDRESS			0x41
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

//Macros for bias calibration
#define		PWR_MGMT_1       0x6B // Device defaults to the SLEEP mode
#define		PWR_MGMT_2       0x6C
#define     INT_ENABLE       0x38
#define		FIFO_EN          0x23
#define		I2C_MST_CTRL     0x24
#define		USER_CTRL        0x6A  // Bit 7 enable DMP, bit 3 reset DMP
#define		CONFIG           0x1A
#define		SMPLRT_DIV       0x19
#define		FIFO_COUNTH      0x72
#define		FIFO_R_W         0x74
#define		XG_OFFSET_H      0x13  // User-defined trim values for gyroscope
#define		XG_OFFSET_L      0x14
#define		YG_OFFSET_H      0x15
#define		YG_OFFSET_L      0x16
#define		ZG_OFFSET_H      0x17
#define		ZG_OFFSET_L      0x18
#define		XA_OFFSET_H      0x06 // User-defined trim values for accelerometer
#define		XA_OFFSET_L_TC   0x07
#define		YA_OFFSET_H      0x08
#define		YA_OFFSET_L_TC   0x09
#define		ZA_OFFSET_H      0x0A
#define		ZA_OFFSET_L_TC   0x0B

enum status_code MPU9250_init(void);
enum status_code gyro_init(void);
enum status_code accel_init(void);
enum status_code mag_init(void);
void convert_MPU9250_to_EQUiSat_coords(int16_t toFill[3]);
enum status_code MPU9250_read_mag(int16_t toFill[3]);
enum status_code MPU9250_read_mag_EQUiSat_coords(int16_t toFill[3]);
enum status_code MPU9250_read_acc(int16_t toFill[3]);
enum status_code MPU9250_read_acc_EQUiSat_coords(int16_t toFill[3]);
enum status_code MPU9250_read_gyro(int16_t toFill[3]);
enum status_code MPU9250_read_gyro_EQUiSat_coords(int16_t toFill[3]);
enum status_code MPU9250_read_temp(int16_t* buf);
void MPU9250_computeBias(float* dest1, float* dest2);
float MPU9250_computeCompassDir(int16_t x, int16_t y, int16_t z);

#endif