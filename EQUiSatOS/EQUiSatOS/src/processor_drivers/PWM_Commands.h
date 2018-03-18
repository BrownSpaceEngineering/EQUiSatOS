/*
 * PWM_COMMANDS.h
 *
 * Created: 12/4/2016 3:55:24 PM
 *  Author: Daniel
 */ 


#ifndef PWM_COMMANDS_H_
#define PWM_COMMANDS_H_

#include <global.h>

void configure_pwm(int pwm_pin, int pwm_mux, int p_ant);
void enable_pwm(int on_fraction);
void disable_pwm(void);

struct tcc_module tcc_instance;
int channel;

//Config parameters
#define CONF_PWM_MODULE      TCC0

//period configuration
#define PWM_PERIOD			0x14
#define PWM_ON_PERIOD		0x8



#endif /* PWM_COMMANDS_H_ */