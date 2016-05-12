#ifndef _MLX90614_IR_SENSOR_H_
#define _MLX90614_IR_SENSOR_H_

#include <I2C_Commands.h>

#define MLX90614_DEFAULT_I2CADDR 0x5A
#define MLX90614_TBOARD_IR2 0x5B // address IR2 on Processor Test Board

//ram addresses
#define MLX90614_RAWIR1 0x04
#define MLX90614_RAWIR2 0x05
#define MLX90614_TA 0x06
#define MLX90614_TOBJ1 0x07
#define MLX90614_TOBJ2 0x08

//eeprom addresses
#define MLX90614_SMBUS 0x2E

i2c_func MLX90614_i2c_write_func;
i2c_func MLX90614_i2c_read_func;
i2c_func MLX90614_i2c_write_no_stop_func;

void MLX90614_init(i2c_func _i2c_write_func, i2c_func _i2c_read_func, i2c_func _i2c_write_no_stop_func);
void read_MLX90614(uint8_t device_addr, uint8_t mem_addr, uint8_t* buf);
uint16_t MLX90614_read2ByteValue(uint16_t device_addr, uint8_t mem_addr, bool lsb_first);
uint16_t MLX90614_readRawIRData(uint8_t device_addr, bool is_ir2);
float dataToTemp(uint16_t data);
float MLX90614_readTempC(uint8_t device_addr, bool is_ambient);

uint16_t MLX90614_getAddress(uint8_t device_addr);

void write_MLX90614_eeprom(uint8_t device_addr, uint8_t mem_addr, uint8_t* buf);
void MLX90614_setAddress(uint8_t current_addr, uint8_t new_addr);

uint8_t crc(uint8_t* message, int nBytes);
#endif
