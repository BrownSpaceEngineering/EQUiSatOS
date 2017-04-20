/*
 * PWM_COMMANDS.h
 *
 * Created: 12/4/2016 3:55:24 PM
 *  Author: Daniel
 */ 


#ifndef PWM_COMMANDS_H_
#define PWM_COMMANDS_H_

#include <global.h>

void configure_pwm(void);
bool setPulseWidthFraction(int numerator, int denominator);

struct tcc_module tcc_instance;

//Config parameters
#define CONF_PWM_MODULE      TCC0
#define CONF_PWM_CHANNEL     2 
#define CONF_PWM_OUTPUT      6
#define CONF_PWM_OUT_PIN     PIN_PA16F_TCC0_WO6
#define CONF_PWM_OUT_MUX     PINMUX_PA16F_TCC0_WO6

//period configuration
#define PWM_PERIOD			0x10
#define PWM_ON_PERIOD		0x8



#endif /* PWM_COMMANDS_H_ */