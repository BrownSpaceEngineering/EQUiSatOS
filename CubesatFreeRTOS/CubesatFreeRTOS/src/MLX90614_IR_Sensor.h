#ifndef _MLX90614_IR_SENSOR_H_
#define _MLX90614_IR_SENSOR_H_

#include <I2C_Commands.h>

#define MLX90614_I2CADDR 0x5A

#define MLX90614_RAWIR1 0x04
#define MLX90614_RAWIR2 0x05
#define MLX90614_TA 0x06
#define MLX90614_TOBJ1 0x07
#define MLX90614_TOBJ2 0x08


void read_MLX90614(uint8_t* buf, uint8_t addr);
float readTemp(uint8_t addr);
float MLX90614_readObjectTempC();
float MLX90614_readAmbientTempC();

#endif
