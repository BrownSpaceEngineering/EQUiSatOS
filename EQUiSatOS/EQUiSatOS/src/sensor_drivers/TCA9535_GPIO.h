/*
 * TCA9535_GPIO.h
 *
 * Created: 3/15/2017 9:17:07 PM
 *  Author: BSE
 */ 


#ifndef TCA9535_GPIO_H_
#define TCA9535_GPIO_H_

#include "../global.h"
#include "../processor_drivers/I2C_Commands.h"

#define TCA_ADDR 0b0100000
#define TCA_SHOULD_STOP_WRITE true
#define TCA_SHOULD_STOP_READ false

#define INPUTS_REGISTER_0 0x0
#define INPUTS_REGISTER_1 0x1
#define OUTPUTS_REGISTER_0 0x2
#define OUTPUTS_REGISTER_1 0x3
#define POLARITY_INVERSION_0 0x4
#define POLARITY_INVERSION_1 0x5
#define CONFIG_REGISTER_0 0x6
#define CONFIG_REGISTER_1 0x7

//0 is output, 1 is input - see battery board v2 datasheet for rationale
#define IOMask0 0b11111111 //Port 0: 0.7->0.0
#define IOMask1 0b11110000 //Port 1: 1.7->1.0


void TCA9535_init(return_struct_16 *rs);
enum status_code setIOMask(uint8_t reg0, uint8_t reg1);
void readTCA9535Levels(return_struct_16 *rs);
enum status_code setIO(bool isArray1, uint8_t char_index_in_register, bool targetLevel);
enum status_code setBatOutputs(uint8_t vals);

#endif /* TCA9535_GPIO_H_ */