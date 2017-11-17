/*
 * LTC1380_Multiplexer_Commands.h
 *
 * Created: 2/12/2017 3:51:29 PM
 *  Author: BSE
 */ 


#ifndef LTC1380_MULTIPLEXER_COMMANDS_H_
#define LTC1380_MULTIPLEXER_COMMANDS_H_

#include "../global.h"
#include "../processor_drivers/I2C_Commands.h"

#define TEMP_MULTIPLEXER_I2C 0x48
//#define PHOTO_MULTIPLEXER_I2C 0x49
#define PHOTO_MULTIPLEXER_I2C 0x4a

void LTC1380_init(void);
enum status_code LTC1380_channel_select(uint8_t addr, uint8_t target, uint8_t* rs);
enum status_code LTC1380_disable(uint8_t addr, uint8_t* rs);

#endif /* LTC1380_MULTIPLEXER_COMMANDS_H_ */