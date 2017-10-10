/*
 * init_rtos_tasks.c
 *
 * Created: 10/4/2016 8:50:22 PM
 *  Author: mckenna
 */

#include "init_rtos_tasks.h"

void run_rtos()
{
	/************************************************************************/
	/* ESSENTIAL INITIALIZATION                                             */
	/************************************************************************/
	//configure_i2c_master(SERCOM4);
	pre_init_rtos_tasks(); // populate task_handles array and setup constants

	// Initialize EQUiStack mutexes
	_last_reading_type_equistack_mutex = xSemaphoreCreateMutexStatic(&_last_reading_type_equistack_mutex_d);
	_idle_equistack_mutex = xSemaphoreCreateMutexStatic(&_idle_equistack_mutex_d);
	_flash_equistack_mutex = xSemaphoreCreateMutexStatic(&_flash_equistack_mutex_d);
	_transmit_equistack_mutex = xSemaphoreCreateMutexStatic(&_transmit_equistack_mutex_d);
	_attitude_equistack_mutex = xSemaphoreCreateMutexStatic(&_attitude_equistack_mutex_d);

	// Initialize EQUiStacks
	equistack_Init(&last_reading_type_equistack, &_last_reading_equistack_arr,
		sizeof(msg_data_type_t), LAST_READING_TYPE_STACK_MAX, &_last_reading_type_equistack_mutex);
	equistack_Init(&idle_readings_equistack, &_idle_equistack_arr,
		sizeof(idle_data_t), IDLE_STACK_MAX, &_idle_equistack_mutex);
	equistack_Init(&flash_readings_equistack, &_flash_equistack_arr,
		sizeof(flash_data_t), FLASH_STACK_MAX, &_flash_equistack_mutex);
 	equistack_Init(&transmit_readings_equistack, &_transmit_equistack_arr,
		sizeof(transmit_data_t), TRANSMIT_STACK_MAX, &_transmit_equistack_mutex);
 	equistack_Init(&attitude_readings_equistack, &_attitude_equistack_arr,
		sizeof(attitude_data_t), ATTITUDE_STACK_MAX, &_attitude_equistack_mutex);

	/************************************************************************/
	/* TASK CREATION                                                        */
	/************************************************************************/

	// Started at boot
	// TODO: Should we even store this handle?
	battery_charging_task_handle = xTaskCreateStatic(battery_charging_task,
		"battery charging action task",
		TASK_BATTERY_CHARGING_STACK_SIZE,
		NULL,
		TASK_BATTERY_CHARGING_PRIORITY,
		&battery_charging_task_stack,
		&battery_charging_task_buffer);

	// TODO: Should we even store this handle?
	antenna_deploy_task_handle = xTaskCreateStatic(antenna_deploy_task,
		"antenna deploy action task",
		TASK_ANTENNA_DEPLOY_STACK_SIZE,
		NULL,
		TASK_ANTENNA_DEPLOY_PRIORITY,
		&antenna_deploy_task_stack,
		&antenna_deploy_task_buffer);

	 // TODO: Should we even store this handle?
	watchdog_task_handle = xTaskCreateStatic(watchdog_task,
		"watchdog task",
		TASK_WATCHDOG_STACK_SIZE,
		NULL,
		TASK_WATCHDOG_STACK_PRIORITY,
		&watchdog_task_stack,
		&watchdog_task_buffer);

	flash_activate_task_handle = xTaskCreateStatic(flash_activate_task,
		"flash action task",
		TASK_FLASH_ACTIVATE_STACK_SIZE,
		NULL,
		TASK_FLASH_ACTIVATE_PRIORITY,
		&flash_activate_task_stack,
		&flash_activate_task_buffer);

	transmit_task_handle = xTaskCreateStatic(transmit_task,
		"transmit action task",
		TASK_TRANSMIT_STACK_SIZE,
		NULL,
		TASK_TRANSMIT_PRIORITY,
		&transmit_task_stack,
		&transmit_task_buffer);

	/* Data tasks */

	current_data_task_handle = xTaskCreateStatic(current_data_task,
		"current data reader task",
		TASK_CURRENT_DATA_RD_STACK_SIZE,
		NULL,
		TASK_CURRENT_DATA_RD_PRIORITY,
		&current_data_task_stack,
		&current_data_task_buffer);

	flash_data_task_handle = xTaskCreateStatic(flash_data_task,
		"flash data reader task",
		TASK_FLASH_DATA_RD_STACK_SIZE,
		NULL,
		TASK_FLASH_DATA_RD_PRIORITY,
		&flash_data_task_stack,
		&flash_data_task_buffer);

	transmit_data_task_handle = xTaskCreateStatic(transmit_data_task,
		"transmission data reader task",
		TASK_TRANSMIT_DATA_RD_STACK_SIZE,
		NULL,
		TASK_TRANSMIT_DATA_RD_PRIORITY,
		&transmit_data_task_stack,
		&transmit_data_task_buffer);

	attitude_data_task_handle = xTaskCreateStatic(attitude_data_task,
		"attitude data reader task",
		TASK_ATTITUDE_DATA_RD_STACK_SIZE,
		NULL,
		TASK_ATTITUDE_DATA_DATA_RD_PRIORITY,
		&attitude_data_task_stack,
		&attitude_data_task_buffer);

// 		xTaskCreate(task_suicide_test,
// 		"task suicide testing",
// 		TASK_SENS_RD_IDLE_STACK_SIZE,
// 		NULL,
// 		TASK_SENS_RD_IDLE_PRIORITY,
// 		suicide_test_handle);

// 		xTaskCreate(task_stack_size_overflow_test,
// 			"task stack size purpose test",
// 			320/sizeof(portSTACK_TYPE), // 10 bytes of stack space
// 			NULL,
// 			tskIDLE_PRIORITY,
// 			suicide_test_handle);

  /*xTaskCreate(test_free,
  		"Tests the freeing of structs",
  		TASK_SENS_RD_IDLE_STACK_SIZE,
  		NULL,
  		TASK_SENS_RD_IDLE_PRIORITY,
  		NULL);*/

  	// Make sure we define the first state
  	set_state_idle();

	/* Start the tasks and timer running. */
	vTaskStartScheduler();
}

void set_state_hello_world()
{
	CurrentState = HELLO_WORLD;

	// run only the attitude data task
	task_resume_if_suspended(BATTERY_CHARGING_TASK); // should never be stopped
	task_suspend(ANTENNA_DEPLOY_TASK); // right
	task_suspend(CURRENT_DATA_TASK);
	task_suspend(FLASH_ACTIVATE_TASK);
	task_suspend(TRANSMIT_TASK);
	task_resume_if_suspended(ATTITUDE_DATA_TASK);
		// TODO: Others
}

void set_state_idle()
{
	CurrentState = IDLE;

	task_resume_if_suspended(BATTERY_CHARGING_TASK); // should never be stopped
	task_resume_if_suspended(ANTENNA_DEPLOY_TASK); // should never be stopped?????
	task_resume_if_suspended(CURRENT_DATA_TASK);
	task_resume_if_suspended(FLASH_ACTIVATE_TASK);
	task_resume_if_suspended(TRANSMIT_TASK);
	task_resume_if_suspended(ATTITUDE_DATA_TASK);
		// TODO: Others
}

void set_state_low_power()
{
	CurrentState = LOW_POWER;

	task_resume_if_suspended(BATTERY_CHARGING_TASK); // should never be stopped
	task_resume_if_suspended(ANTENNA_DEPLOY_TASK); // should never be stopped?????
	task_resume_if_suspended(CURRENT_DATA_TASK);
	task_suspend(FLASH_ACTIVATE_TASK);
	task_resume_if_suspended(TRANSMIT_TASK);
	task_suspend(ATTITUDE_DATA_TASK); // TODO: Do this?
	// TODO: Others
}