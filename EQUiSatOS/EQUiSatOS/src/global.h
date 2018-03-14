/*
 * global.h
 *
 * Created: 4/12/2017 9:35:47 PM
 *  Author: jleiken
 */


#ifndef GLOBAL_H_
#define GLOBAL_H_

#include <asf.h>
#include <inttypes.h>
#include <stdarg.h>
#include <config.h>
#include <errors.h>

#include "FreeRTOS.h"
#include "config/proc_pins.h"
#include "processor_drivers/I2C_Commands.h"
#include "processor_drivers/MRAM_Commands.h"
#include "processor_drivers/ADC_Commands.h"
#include "processor_drivers/Direct_Pin_Commands.h"
#include "processor_drivers/USART_Commands.h"
#include "processor_drivers/PWM_Commands.h"
#include "processor_drivers/Watchdog_Commands.h"
#include "processor_drivers/RTC_Commands.h"
#include "sensor_drivers/MLX90614_IR_Sensor.h"
#include "sensor_drivers/TEMD6200_Commands.h"
#include "sensor_drivers/switching_commands.h"
#include "sensor_drivers/MPU9250_9axis_Commands.h"
#include "sensor_drivers/LTC1380_Multiplexer_Commands.h"
#include "sensor_drivers/AD7991_ADC.h"
#include "sensor_drivers/TCA9535_GPIO.h"
#include "sensor_drivers/HMC5883L_Magnetometer_Commands.h"
#include "sensor_drivers/M24M01_EEPROM_Commands.h"
#include "rtos_tasks/rtos_tasks_config.h"
#include "rtos_tasks/transmit_task.h"
#include "data_handling/Sensor_Structs.h"
#include "telemetry/Radio_Commands.h"
#include "testing_functions/system_test.h"
#include "sensor_drivers/sensor_read_commands.h"
#include "runnable_configurations/satellite_state_control.h"

/************************************************************************/
/* HELPER METHODS                                                       */
/************************************************************************/
void init_tracelyzer(void);
void global_init(void);
void global_init_post_rtos(void);
void suppress_other_prints(bool on);
void print(const char *format, ...);

#if PRINT_DEBUG > 0 && defined(SAFE_PRINT)
	// print mutex; used to both lock the print buffer and prevent USART contention while printing
	#define PRINT_MUTEX_WAIT_TIME_TICKS				((TickType_t) 2000 / portTICK_PERIOD_MS) // prints (or print batches) can take a while
	SemaphoreHandle_t print_mutex;
#endif

#if configUSE_TRACE_FACILITY == 1
	traceString global_trace_channel;
#endif

#if configUSE_TRACE_FACILITY == 1
	// http://www.delorie.com/gnu/docs/gcc/gcc_44.html
	#define trace_print(format, ...) vTracePrintF(global_trace_channel, format, ##__VA_ARGS__);
#else
	#define trace_print(...) ((void)0);
#endif

#endif /* GLOBAL_H_ */
