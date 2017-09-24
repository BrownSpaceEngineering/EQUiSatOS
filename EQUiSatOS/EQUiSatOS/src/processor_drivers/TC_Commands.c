/*
 * TC_Commands.c
 *
 * Created: 9/17/2017 3:23:42 PM
 *  Author: rj16
 *
 * Code adapted from 
 * http://www.atmel.com/Images/Atmel-42123-SAM-Timer-Counter-%28TC%29-Driver_ApplicationNote_AT03263.pdf
 *
 */ 

#include "TC_Commands.h"

void init_tc(void)
{
	static int retries = 0;
	
	struct tc_config config_tc;
	tc_get_config_defaults(&config_tc);
	
	config_tc.counter_size = TC_COUNTER_SIZE_32BIT;
	config_tc.count_direction = TC_COUNT_DIRECTION_UP;
	config_tc.counter_32_bit.compare_capture_channel[0] = (0xFFFF / 4);	config_tc.clock_prescaler = TC_CLOCK_PRESCALER_DIV1024;
	
	enum status_code code = tc_init(&tc_instance, EXT1_PWM_MODULE, &config_tc);

	if (code != STATUS_OK)
	{
		if (code == STATUS_BUSY && retries <= 2)
		{
			retries++;
			init_tc();
		}
		else
		{
			// TODO: Kick watchdog here
		}
	}
	
	tc_enable(&tc_instance);
	int count = get_count();
	int x;
}int get_count(){	return tc_get_count_value(&tc_instance);}// Just an eye testvoid test_timer() {	int last_count = -1;	int count = 0;		while (true) 	{		count = get_count();	}}