/*
 * IncFile1.h
 *
 * Created: 4/26/2015 3:39:03 PM
 *  Author: Daniel
 */ 


#ifndef HMC5883L_MAGNETOMETER_COMMANDS_H
#define HMC5883L_MAGNETOMETER_COMMANDS_H
#include "../processor_drivers/I2C_Commands.h"
#include <global.h>
#include <math.h>

#define HMC5883L_ADDRESS 0x1E

enum status_code HMC5883L_init(void);
enum status_code HMC5883L_read(uint8_t* read_buffer);
enum status_code HMC5883L_readXYZ(int16_t* read_buffer);
float HMC5883L_computeCompassDir(int16_t x, int16_t y, int16_t z);

#endif /* INCFILE1_H_ */
