/*
 * IncFile1.h
 *
 * Created: 4/26/2015 3:39:03 PM
 *  Author: Daniel
 */ 


#ifndef HMC5883L_MAGNETOMETER_COMMANDS_H
#define HMC5883L_MAGNETOMETER_COMMANDS_H
#include "../processor_drivers/I2C_Commands.h"
#include <asf.h>

#define HMC5883L_ADDRESS 0x1E

i2c_func HMC5883L_i2c_write_func;
i2c_func HMC5883L_i2c_read_func;

void HMC5883L_init(i2c_func _i2c_write_func, i2c_func _i2c_read_func);
void HMC5883L_read(uint8_t* read_buffer);
void HMC5883L_getXYZ(uint8_t* readBuffer, int16_t* xyzBuffer);
float HMC5883L_computeCompassDir(int16_t x, int16_t y, int16_t z);

#endif /* INCFILE1_H_ */
