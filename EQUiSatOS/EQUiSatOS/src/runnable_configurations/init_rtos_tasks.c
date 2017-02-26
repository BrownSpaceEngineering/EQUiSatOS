/*
 * init_rtos_tasks.c
 *
 * Created: 10/4/2016 8:50:22 PM
 *  Author: mckenna
 */

#include "init_rtos_tasks.h"

#include "task_testing.h"

void runit_2()
{
	//configure_i2c_master(SERCOM4);

	// check that constants
	assertConstantDefinitions();

	// Initialize EQUiStacks
	idle_readings_equistack = idle_Stack_Init();
	flash_readings_equistack = flash_Stack_Init();

	// Started at boot
	xTaskCreate(battery_charging_task,
		"battery charging action task",
		TASK_RADIO_TRANSMIT_STACK_SIZE,
		NULL,
		TASK_RADIO_TRANSMIT_PRIORITY,
		battery_charging_task_handle);

	xTaskCreate(antenna_deploy_task,
		"antenna deploy action task",
		TASK_ANTENNA_DEPLOY_STACK_SIZE,
		NULL,
		TASK_ANTENNA_DEPLOY_PRIORITY,
		antenna_deploy_task_handle);

	// TODO: Will these not be created here?
	// i.e. would they be created when they need to be started?
	// Would that work after the scheduler has been started?
	xTaskCreate(watchdog_task,
		"watchdog task",
		TASK_WATCHDOG_STACK_SIZE,
		NULL,
		TASK_WATCHDOG_STACK_PRIORITY,
		watchdog_task_handle);

	xTaskCreate(flash_activate_task,
		"flash action task",
		TASK_FLASH_ACTIVATE_STACK_SIZE,
		NULL,
		TASK_FLASH_ACTIVATE_PRIORITY,
		flash_activate_task_handle);

	xTaskCreate(transmit_task,
		"transmit action task",
		TASK_TRANSMIT_STACK_SIZE,
		NULL,
		TASK_TRANSMIT_BOOT_PRIORITY,
		transmit_task_handle);

	xTaskCreate(current_data_task,
		"current data reader task",
		TASK_CURRENT_DATA_RD_STACK_SIZE,
		NULL,
		TASK_CURRENT_DATA_RD_PRIORITY,
		current_data_task_handle);

	xTaskCreate(current_data_low_power_task,
		"current data low power reader task",
		TASK_CURRENT_DATA_LOW_POWER_RD_STACK_SIZE,
		NULL,
		TASK_CURRENT_DATA_LOW_POWER_RD_PRIORITY,
		current_data_low_power_task_handle);

	xTaskCreate(attitude_data_task,
		"attitude data reader task",
		TASK_ATTITUDE_DATA_RD_STACK_SIZE,
		NULL,
		TASK_ATTITUDE_DATA_DATA_RD_PRIORITY,
		attitude_data_task_handle);

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

	xTaskCreate(attitude_data_task,
		"attitude data reader task",
		TASK_ATTITUDE_DATA_RD_STACK_SIZE,
		NULL,
		TASK_ATTITUDE_DATA_DATA_RD_PRIORITY,
		attitude_data_task_handle);
	/*vTaskResume(attitude_data_task_handle); // TODO: Does this START the task? Or should we just do the above?*/
}

void set_state_idle()
{
	CurrentState = IDLE;

	// TODO: we need to suspend the other tasks and somehow immediately add OR delete their interior structs and make a new one
	// Maybe look for changes in state inside the rtos tasks?
	// OR bring their current structs, etc. global so we can manually reset them? -> NOOOOO

	vTaskResume(current_data_task_handle);
	vTaskResume(flash_activate_task_handle);
	vTaskSuspend(current_data_low_power_task_handle);
}

void set_state_low_power()
{
	CurrentState = LOW_POWER;

	// TODO: ibid
	vTaskResume(current_data_low_power_task_handle);
	vTaskSuspend(current_data_task_handle);
	vTaskSuspend(flash_activate_task_handle);
}
