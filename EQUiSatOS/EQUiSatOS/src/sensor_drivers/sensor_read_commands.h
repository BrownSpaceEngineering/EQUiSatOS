/*
 * sensor_read_commands.h
 *
 * Created: 11/1/2016 8:17:44 PM
 *  Author: mckenna
 */ 

#ifndef SENSOR_READ_COMMANDS_H
#define	SENSOR_READ_COMMANDS_H

#include "../stacks/Sensor_Structs.h"

#include "processor_drivers/I2C_Commands.h"
#include "processor_drivers/SPI_Commands.h"
#include "processor_drivers/ADC_Commands.h"
#include "processor_drivers/Direct_Pin_Commands.h"

//#include "sensor_drivers/HMC5883L_Magnetometer_Commands.h"
#include "sensor_drivers/MLX90614_IR_Sensor.h"
#include "sensor_drivers/TEMD6200_Commands.h"
#include "sensor_drivers/switching_commands.h"
#include "sensor_drivers/sensor_read_commands.h"

/**
 * Helpful regex: (\w*)_DATA -> $1_batch read_$1_batch(void);
 */
read_lion_volts_batch(lion_volts_batch batch);
read_lion_current_batch(lion_current_batch batch);
read_led_temps_batch(led_temps_batch batch);
read_lifepo_current_batch(lifepo_current_batch batch);
read_lifepo_volts_batch(lifepo_volts_batch batch);
read_ir_batch(ir_batch batch);
read_diode_batch(diode_batch batch);
read_bat_temp_batch(bat_temp_batch batch);
read_ir_temps_batch(ir_temps_batch batch);
read_radio_temp_batch(radio_temp_batch batch);
read_imu_batch(imu_batch *batch); // only one that's a struct not an array ref, so make sure it passes by reference too
read_magnetometer_batch(magnetometer_batch batch);
read_led_current_batch(led_current_batch batch);
read_radio_volts_batch(radio_volts_batch batch);
read_bat_charge_volts_batch(bat_charge_volts_batch batch);
read_bat_charge_dig_sigs_batch(bat_charge_dig_sigs_batch batch);
read_digital_out_batch(digital_out_batch batch);

#endif