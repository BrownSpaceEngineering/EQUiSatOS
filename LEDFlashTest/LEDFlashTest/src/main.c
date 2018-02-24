/**
 * \file
 *
 * \brief Test that the bootloader can write a program to MRAM, then load a program from MRAM
 *
 */

/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */
/*
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */
#include <asf.h>
#include <port.h>
#include <delay.h>

#define TEST_DELAY 1000 // CHANGE THIS

int main (void)
{
	system_init();

	const uint8_t test_pin = PIN_PA13; // - ANT_DRV1 (via) //PIN_PA28 //PIN_PA24; - main board
	struct port_config conf;
	port_get_config_defaults(&conf);
	conf.direction = PORT_PIN_DIR_OUTPUT;
	port_pin_set_config(test_pin, &conf);
	port_pin_set_output_level(test_pin, false);
	
	delay_init();
	
	while (true) {
		port_pin_set_output_level(test_pin, true);
		delay_ms(TEST_DELAY);
		port_pin_set_output_level(test_pin, false);
		delay_ms(TEST_DELAY);
		
	}
	
	return 0;
}
