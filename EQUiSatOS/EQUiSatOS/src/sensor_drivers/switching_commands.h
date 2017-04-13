/*
 * IncFile1.h
 *
 * Created: 5/8/2015 3:20:25 PM
 *  Author: Daniel
 */ 


#ifndef SWITCHING_COMMANDS_H
#define SWITCHING_COMMANDS_H

#include <global.h>
#include "../processor_drivers/Direct_Pin_Commands.h"

#define SIDE_1_ENABLE PIN_PA27
#define SIDE_2_ENABLE PIN_PA28

#define BIT_0_CONTROL PIN_PB22
#define BIT_1_CONTROL PIN_PB23

#define OUTPUT PIN_PB00

void pick_side(bool targetSide);
void pick_input(uint8_t target);
void setup_switching(void);

#endif /* SWITCHING_COMMANDS_H */