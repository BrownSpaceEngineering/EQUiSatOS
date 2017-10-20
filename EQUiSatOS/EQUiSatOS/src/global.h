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

#include "config/proc_pins.h"
#include "processor_drivers/I2C_Commands.h"
#include "processor_drivers/SPI_Commands.h"
#include "processor_drivers/ADC_Commands.h"
#include "processor_drivers/Direct_Pin_Commands.h"
#include "processor_drivers/USART_Commands.h"
#include "processor_drivers/PWM_Commands.h"
#include "processor_drivers/Watchdog_Commands.h"
#include "processor_drivers/TC_Commands.h"
#include "sensor_drivers/MLX90614_IR_Sensor.h"
#include "sensor_drivers/TEMD6200_Commands.h"
#include "sensor_drivers/switching_commands.h"
#include "sensor_drivers/MPU9250_9axis_Commands.h"
#include "sensor_drivers/LTC1380_Multiplexer_Commands.h"
#include "sensor_drivers/AD7991_ADC.h"
#include "sensor_drivers/TCA9535_GPIO.h"
#include "telemetry/Radio_Commands.h"
#include "errors.h"

/* Global satellite state */
int8_t CurrentState;

typedef struct return_struct_float {
	enum status_code return_status;
	float return_value;
} return_struct_float;

typedef struct return_struct_16 {
	enum status_code return_status;
	uint16_t return_value;
} return_struct_16;
	
typedef struct return_struct_8 {
	enum status_code return_status;
	uint8_t return_value;
} return_struct_8;
	
typedef struct return_struct_0 {
	enum status_code return_status;
} return_struct_0;


#endif /* GLOBAL_H_ */