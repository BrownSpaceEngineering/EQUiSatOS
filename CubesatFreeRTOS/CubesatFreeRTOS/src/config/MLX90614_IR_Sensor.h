/*
 * MLX9061_IR_Sensor.h
 *
 * Created: 30.09.2015 21:47
 *  Author: Arun
 */

#ifndef _MLX90614_IR_SENSOR_H_
#define _MLX90614_IR_SENSOR_H_

#include <I2C_Commands.h>

#define MLX90614_WRITE_ADDR 0x5A
#define MLX90614_READ_ADDR 0x5B

#define TA_ADDR 0x006
#define TOBJ1_ADDR 0x007
#define TOBJ2_ADDR 0x008

void MLX90614_read(uint8_t* buf, uint8_t addr);
void MLX90614_write(uint8_t* buf);

#endif
