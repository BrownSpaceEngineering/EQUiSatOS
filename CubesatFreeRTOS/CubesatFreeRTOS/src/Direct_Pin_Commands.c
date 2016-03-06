#include <Direct_Pin_Commands.h>

void get_pin(bool dir,uint8_t pin){
	struct port_config conf;
	port_get_config_defaults(&conf);
	if(dir){
		conf.direction = PORT_PIN_DIR_OUTPUT;
	}else{
		conf.direction = PORT_PIN_DIR_INPUT;
	}
	port_pin_set_config(pin,&conf);
}

void set_output(bool output, uint8_t pin){
	port_pin_set_output_level(pin,output);
}

bool get_input(uint8_t pin){
	return port_pin_get_input_level(pin);
}