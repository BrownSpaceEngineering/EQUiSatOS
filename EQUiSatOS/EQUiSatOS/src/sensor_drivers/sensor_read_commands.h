/*
 * sensor_read_commands.h
 *
 * Created: 11/1/2016 8:17:44 PM
 *  Author: mckenna
 */ 

#ifndef SENSOR_READ_COMMANDS_H
#define	SENSOR_READ_COMMANDS_H

#include "../global.h"

/**
 * Helpful regex: (\w*)_DATA -> $1_batch read_$1_batch(void);
 */
void read_lion_volts_batch(			lion_volts_batch batch);
void read_lion_current_batch(		lion_current_batch batch);
void read_led_temps_batch(			led_temps_batch batch);
void read_lifepo_current_batch(		lifepo_current_batch batch);
void read_lifepo_volts_batch(		lifepo_volts_batch batch);
void read_ir_batch(					ir_batch batch);
void read_pdiode_batch(				pdiode_batch batch);
void read_bat_temp_batch(			bat_temp_batch batch);
void read_ir_temps_batch(			ir_temps_batch batch);
void read_radio_temp_batch(			radio_temp_batch* batch);			// pointer to single value
void read_accel_batch(				accelerometer_batch accel_batch);
void read_gyro_batch(				gyro_batch gyr_batch);
void read_magnetometer_batch(		magnetometer_batch batch);
void read_led_current_batch(		led_current_batch batch);
void read_radio_volts_batch(		radio_volts_batch batch);
void read_bat_charge_volts_batch(	bat_charge_volts_batch batch);
void read_bat_charge_dig_sigs_batch(bat_charge_dig_sigs_batch* batch);	// pointer to single value
void read_digital_out_batch(		digital_out_batch* batch);			// pointer to single value
void read_imu_temp_batch(			imu_temp_batch* batch);				// pointer to single value
void read_rail_3v_batch(			rail_3v_batch* batch);				// pointer to single value
void read_rail_5v_batch(			rail_5v_batch* batch);				// pointer to single value

#endif