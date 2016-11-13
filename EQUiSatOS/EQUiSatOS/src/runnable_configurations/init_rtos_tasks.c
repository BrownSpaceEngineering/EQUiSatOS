/*
 * init_rtos_tasks.c
 *
 * Created: 10/4/2016 8:50:22 PM
 *  Author: mckenna
 */ 

#include "init_rtos_tasks.h"
#include "rtos_tasks.h"

void runit_2()
{
	//configure_i2c_master(SERCOM4);
	
	// Initialize EQUiStacks
	//idle_Stack_Init(idle_readings_equistack)
	
	// Make sure we define the first state
	set_state_idle();
	
	xTaskCreate(task_data_read_idle,
			"idle data reader",
			TASK_SENS_RD_IDLE_STACK_SIZE,
			NULL,
			TASK_SENS_RD_IDLE_PRIORITY,
			NULL);
			
	xTaskCreate(test_free,
			"Tests the freeing of structs",
			TASK_SENS_RD_IDLE_STACK_SIZE,
			NULL,
			TASK_SENS_RD_IDLE_PRIORITY,
			NULL);
			
	/* Start the tasks and timer running. */
	vTaskStartScheduler();
}

void set_state_idle()
{
	CurrentState = IDLE;
	
	taskFrequencies[LED_TASK] =					idle_TASK_LED_FREQ;
	taskFrequencies[RADIO_TRANSMIT_TASK] =		idle_TASK_RADIO_TRANSMIT_FREQ;
}

void set_state_flash()
{
	CurrentState = FLASH;
	
	taskFrequencies[LED_TASK] =					flash_TASK_LED_FREQ;
	taskFrequencies[RADIO_TRANSMIT_TASK] =		flash_TASK_RADIO_TRANSMIT_FREQ;
}

void set_state_low_power()
{
	CurrentState = LOW_POWER;
	
	taskFrequencies[LED_TASK] =					low_power_TASK_LED_FREQ;
	taskFrequencies[RADIO_TRANSMIT_TASK] =		low_power_TASK_RADIO_TRANSMIT_FREQ;
}