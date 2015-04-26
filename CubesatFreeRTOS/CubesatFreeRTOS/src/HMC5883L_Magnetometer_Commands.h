/*
 * IncFile1.h
 *
 * Created: 4/26/2015 3:39:03 PM
 *  Author: Daniel
 */ 


#ifndef HMC5883L_MAGNETOMETER_COMMANDS_H
#define HMC5883L_MAGNETOMETER_COMMANDS_H

#include <I2C_Commands.h>

#define HMC5883L_ADDRESS 0x1E

void HMC5883L_init(void);
void HMC5883L_read(uint8_t* read_buffer);

#endif /* INCFILE1_H_ */