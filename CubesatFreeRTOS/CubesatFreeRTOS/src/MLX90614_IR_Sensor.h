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

#define RAW_DATA_IR_CHAN1 0x004
#define RAW_DATA_IR_CHAN2 0x005
#define TA_ADDR 0x06
#define TOBJ_ADDR 0x07

void MLX90614_read(uint8_t* buf, uint8_t addr);
void MLX90614_write(uint8_t* buf);

float MLX90614_read_temperature();
float calcTemperature(int16_t rawTemp);

#endif
