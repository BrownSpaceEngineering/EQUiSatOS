#ifndef _MLX90614_IR_SENSOR_H_
#define _MLX90614_IR_SENSOR_H_

#include "../processor_drivers/I2C_Commands.h"
#include "../processor_drivers/Direct_Pin_Commands.h"

#define MLX90614_DEFAULT_I2CADDR 0x5A
#define MLX90614_TBOARD_IR2 0x5B // address IR2 on Processor Test Board
#define MLX90614_FLASH_PANEL_B1 0X6C //ADRESS on flash panel v6.2 b1

#define POWER_PIN P_IR_PWR_CMD

#define MLX90614_SHOULD_STOP false

#define MLX90614_WHOAMI	0x2e

//ram addresses
#define MLX90614_RAWIR1 0x04
#define MLX90614_RAWIR2 0x05
#define MLX90614_TA 0x06
#define MLX90614_TOBJ1 0x07
#define MLX90614_TOBJ2 0x08

//eeprom addresses
#define MLX90614_SMBUS 0x2E

typedef enum {DEFAULT = MLX90614_DEFAULT_I2CADDR,
              TBOARD_IR2 = MLX90614_TBOARD_IR2,
			  FLASH_PANEL_B1 = MLX90614_FLASH_PANEL_B1
              /* TODO: add more members to this enum
                 as we get more IR sensors to talk to */
              } MLXDeviceAddr;

typedef enum {IR1 = MLX90614_RAWIR1,
             IR2 = MLX90614_RAWIR2} IRChannel;

typedef enum {AMBIENT = MLX90614_TA,
              OBJ1 = MLX90614_TOBJ1,
              OBJ2 = MLX90614_TOBJ2} IRTempTarget;

void MLX90614_init(void);
uint16_t MLX90614_read2ByteValue(MLXDeviceAddr addr, uint8_t mem_addr);
uint16_t MLX90614_readRawIRData(MLXDeviceAddr addr, IRChannel chan);
float dataToTemp(uint16_t data);
float MLX90614_readTempC(MLXDeviceAddr addr, IRTempTarget temp_target);

uint16_t MLX90614_getAddress(MLXDeviceAddr addr);

void irPower(bool trueIsOn);
bool isIROn(void);

#endif
