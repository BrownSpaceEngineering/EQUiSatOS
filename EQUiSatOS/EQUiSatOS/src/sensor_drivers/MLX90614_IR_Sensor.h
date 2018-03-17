#ifndef _MLX90614_IR_SENSOR_H_
#define _MLX90614_IR_SENSOR_H_

#include "../config/proc_pins.h"
#include "../processor_drivers/I2C_Commands.h"
#include "../processor_drivers/Direct_Pin_Commands.h"

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
              TBOARD_IR2 = MLX90614_TBOARD_IR2
              } MLXDeviceAddr;

typedef enum {IR1 = MLX90614_RAWIR1,
             IR2 = MLX90614_RAWIR2} IRChannel;

typedef enum {AMBIENT = MLX90614_TA,
              OBJ1 = MLX90614_TOBJ1,
              OBJ2 = MLX90614_TOBJ2} IRTempTarget;

void MLX90614_init(void);
enum status_code MLX90614_read_all_obj(MLXDeviceAddr addr, uint16_t* buf);
enum status_code MLX90614_read_amb(MLXDeviceAddr addr, uint16_t* buf);
enum status_code MLX90614_read2ByteValue(MLXDeviceAddr addr, uint8_t mem_addr, uint16_t* buf);
enum status_code MLX90614_readRawIRData(MLXDeviceAddr addr, IRChannel chan, uint16_t* buf);
float dataToTemp(uint16_t data);
enum status_code MLX90614_readTempC(MLXDeviceAddr addr, IRTempTarget temp_target, float* buf);

enum status_code MLX90614_getAddress(MLXDeviceAddr addr, uint16_t* rs);

enum status_code MLX90614_setSleepMode(MLXDeviceAddr addr);

void irPower(bool trueIsOn);
bool isIROn(void);

#endif
