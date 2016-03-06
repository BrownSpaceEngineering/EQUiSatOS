#ifndef _MLX90614_IR_SENSOR_H_
#define _MLX90614_IR_SENSOR_H_

#include <I2C_Commands.h>

/* Opcode bitflags: */
#define MLX90614_EEPROM_OP 0x20
#define MLX90614_RAM_OP 0x00

/* MLX90614 address(es): */
#define MLX90614_I2CADDR 0x5A

/* MLX90614 EEPROM addresses: */
#define MLX90614_EEPROM_SMBUS (0x0E | MLX90614_EEPROM_OP)

/* MLX90614 RAM addresses: */
#define MLX90614_RAWIR1 (0x04 | MLX90614_RAM_OP)
#define MLX90614_RAWIR2 (0x05 | MLX90614_RAM_OP)
#define MLX90614_TA (0x06 | MLX90614_RAM_OP)
#define MLX90614_TOBJ1 (0x07 | MLX90614_RAM_OP)
#define MLX90614_TOBJ2 (0x08 | MLX90614_RAM_OP)

void MLX90614_read(uint8_t SMBus_addr, uint8_t ram_addr, uint8_t* buf);
uint16_t MLX90614_readRawIRData(uint8_t SMBus_addr, uint8_t channel);
float MLX90614_readTemp(uint8_t SMBus_addr, uint8_t ram_addr);
float MLX90614_readObjectTempC(uint8_t SMBus_addr);
float MLX90614_readAmbientTempC(uint8_t SMBus_addr);
uint16_t MLX90614_getSMBusAddr(uint8_t device);

/* UNUSED. Could be used to poll both raw IR channels without
   specifiying. */
struct mlxRawData {uint16_t IR_chan1; uint16_t IR_chan2;};

#endif
