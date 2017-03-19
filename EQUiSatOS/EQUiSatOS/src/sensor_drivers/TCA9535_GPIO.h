/*
 * TCA9535_GPIO.h
 *
 * Created: 3/15/2017 9:17:07 PM
 *  Author: BSE
 */ 


#ifndef TCA9535_GPIO_H_
#define TCA9535_GPIO_H_

#include <asf.h>
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

//0 is output, 1 is input
#define IOMask0 0b11111111
#define IOMask1 0b00000000


void TCA9535_init();
void setIOMask(uint8_t reg0, uint8_t reg1);
uint16_t readTCA9535Levels(void);
void setIO(bool isArray1, uint8_t char_index_in_register, bool targetLevel);

#endif /* TCA9535_GPIO_H_ */