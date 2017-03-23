/*
 * sensor_read_commands.h
 *
 * Created: 11/1/2016 8:17:44 PM
 *  Author: mckenna
 */ 

#ifndef SENSOR_READ_COMMANDS_H
#define	SENSOR_READ_COMMANDS_H

#include "../stacks/Sensor_Structs.h"

/**
 * Helpful regex: (\w*)_DATA -> $1_batch read_$1_batch(void);
 */
lion_volts_batch read_lion_volts_batch(void);
lion_current_batch read_lion_current_batch(void);
led_temps_batch read_led_temps_batch(void);
lifepo_current_batch read_lifepo_current_batch(void);
lifepo_volts_batch read_lifepo_volts_batch(void);
ir_batch read_ir_batch(void);
diode_batch read_diode_batch(void);
bat_temp_batch read_bat_temp_batch(void);
ir_temps_batch read_ir_temps_batch(void);
radio_temp_batch read_radio_temp_batch(void);
imu_batch read_imu_batch(void);
magnetometer_batch read_magnetometer_batch(void);
led_current_batch read_led_current_batch(void);
radio_volts_batch read_radio_volts_batch(void);
bat_charge_volts_batch read_bat_charge_volts_batch(void);
bat_charge_dig_sigs_batch read_bat_charge_dig_sigs_batch(void);
digital_out_batch read_digital_out_batch(void);

#endif