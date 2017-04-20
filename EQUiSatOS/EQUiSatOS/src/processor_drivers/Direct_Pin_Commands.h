/*
 * Direct_Pin_Commmands.h
 *
 * Created: 4/26/2015 3:34:57 PM
 *  Author: Daniel
 */ 


#ifndef DIRECT_PIN_COMMANDS
#define DIRECT_PIN_COMMANDS

#include <global.h>

/*
dir is a boolean, true if write, false if read
pin is a pin definition, such as PIN_PA10
*/
void setup_pin(bool dir,uint8_t pin);
void set_output(bool output, uint8_t pin);
bool get_output(uint8_t pin);
bool get_input(uint8_t pin);

#endif /* INCFILE1_H_ */
