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
	//configure_i2c_master(SERCOM2);
	
	xTaskCreate(task_state_read,
			(char) LED_TASK, // use these so we can reference by it?
			TASK_STATE_RD_STACK_SIZE,
			NULL,
			TASK_STATE_RD_PRIORITY,
			NULL);
}

void set_state_idle()
{
	taskFrequencies[LED_TASK] =					idle_TASK_LED_FREQ;
	taskFrequencies[RADIO_TRANSMIT_TASK] =		idle_TASK_RADIO_TRANSMIT_FREQ;
	taskFrequencies[STATE_READ_TASK] =			idle_TASK_STATE_RD_FREQ;
	taskFrequencies[IR_READ_TASK] =				idle_TASK_IR_RD_FREQ;
	taskFrequencies[TEMP_READ_TASK] =			idle_TASK_TEMP_RD_FREQ;
	taskFrequencies[DIODE_READ_TASK] =			idle_TASK_DIODE_RD_FREQ;
	taskFrequencies[LED_CURRENT_READ_TASK] =	idle_TASK_LED_CURRENT_RD_FREQ;
	taskFrequencies[GYRO_READ_TASK] =			idle_TASK_GYRO_RD_FREQ;
	taskFrequencies[MAGNETOMETER_READ_TASK] =	idle_TASK_MAGNETOMETER_RD_FREQ;
	taskFrequencies[CHARGING_DATA_READ_TASK] =	idle_TASK_CHARGING_DATA_RD_FREQ;
	taskFrequencies[RADIO_TEMP_READ_TASK] =		idle_TASK_RADIO_TEMP_RD_FREQ;
	taskFrequencies[BAT_VOLTAGE_READ_TASK] =	idle_TASK_BAT_VOLTAGE_RD_FREQ;
	taskFrequencies[REG_VOLTAGE_READ_TASK] =	idle_TASK_REG_VOLTAGE_RD_FREQ;
}

void set_state_flash()
{
	taskFrequencies[LED_TASK] =						flash_TASK_LED_FREQ;
	taskFrequencies[RADIO_TRANSMIT_TASK] =			flash_TASK_RADIO_TRANSMIT_FREQ;
	taskFrequencies[STATE_READ_TASK] =				flash_TASK_STATE_RD_FREQ;
	taskFrequencies[IR_READ_TASK] =					flash_TASK_IR_RD_FREQ;
	taskFrequencies[TEMP_READ_TASK] =				flash_TASK_TEMP_RD_FREQ;
	taskFrequencies[DIODE_READ_TASK] =				flash_TASK_DIODE_RD_FREQ;
	taskFrequencies[LED_CURRENT_READ_TASK] =		flash_TASK_LED_CURRENT_RD_FREQ;
	taskFrequencies[GYRO_READ_TASK] =				flash_TASK_GYRO_RD_FREQ;
	taskFrequencies[MAGNETOMETER_READ_TASK] =		flash_TASK_MAGNETOMETER_RD_FREQ;
	taskFrequencies[CHARGING_DATA_READ_TASK] =		flash_TASK_CHARGING_DATA_RD_FREQ;
	taskFrequencies[RADIO_TEMP_READ_TASK] =			flash_TASK_RADIO_TEMP_RD_FREQ;
	taskFrequencies[BAT_VOLTAGE_READ_TASK] =		flash_TASK_BAT_VOLTAGE_RD_FREQ;
	taskFrequencies[REG_VOLTAGE_READ_TASK] =		flash_TASK_REG_VOLTAGE_RD_FREQ;
}

void set_state_low_power()
{
	taskFrequencies[LED_TASK] =						low_power_TASK_LED_FREQ;
	taskFrequencies[RADIO_TRANSMIT_TASK] =			low_power_TASK_RADIO_TRANSMIT_FREQ;
	taskFrequencies[STATE_READ_TASK] =				low_power_TASK_STATE_RD_FREQ;
	taskFrequencies[IR_READ_TASK] =					low_power_TASK_IR_RD_FREQ;
	taskFrequencies[TEMP_READ_TASK] =				low_power_TASK_TEMP_RD_FREQ;
	taskFrequencies[DIODE_READ_TASK] =				low_power_TASK_DIODE_RD_FREQ;
	taskFrequencies[LED_CURRENT_READ_TASK] =		low_power_TASK_LED_CURRENT_RD_FREQ;
	taskFrequencies[GYRO_READ_TASK] =				low_power_TASK_GYRO_RD_FREQ;
	taskFrequencies[MAGNETOMETER_READ_TASK] =		low_power_TASK_MAGNETOMETER_RD_FREQ;
	taskFrequencies[CHARGING_DATA_READ_TASK] =		low_power_TASK_CHARGING_DATA_RD_FREQ;
	taskFrequencies[RADIO_TEMP_READ_TASK] =			low_power_TASK_RADIO_TEMP_RD_FREQ;
	taskFrequencies[BAT_VOLTAGE_READ_TASK] =		low_power_TASK_BAT_VOLTAGE_RD_FREQ;
	taskFrequencies[REG_VOLTAGE_READ_TASK] =		low_power_TASK_REG_VOLTAGE_RD_FREQ;
}