/*
 * sensor_read_commands.h
 *
 * Created: 11/1/2016 8:17:44 PM
 *  Author: mckenna
 */

#ifndef SENSOR_READ_COMMANDS_H
#define	SENSOR_READ_COMMANDS_H

#include "../global.h"

#define IR_OBJ_LOW					0
#define IR_OBJ_HIGH					~0
#define IR_AMB_LOW					0
#define IR_AMB_HIGH					~0
#define PD_LOW						0
#define PD_HIGH						~0
#define LED_TEMP_LOW				0
#define LED_TEMP_HIGH				~0
#define LED_CUR_LOW					0
#define LED_CUR_HIGH				~0
#define L_TEMP_LOW					0
#define L_TEMP_HIGH					~0
#define LF_TEMP_LOW					0
#define LF_TEMP_HIGH				~0
#define LF_CUR_LOW					0
#define LF_CUR_HIGH					~0
#define L_CUR_LOW					0
#define L_CUR_HIGH					~0
#define LF_VOLT_LOW					0
#define LF_VOLT_HIGH				~0
#define L_VOLT_LOW					0
#define L_VOLT_HIGH					~0
#define CHARGE_LOW					0
#define CHARGE_HIGH					~0
#define GYRO_LOW					0
#define GYRO_HIGH					~0
#define RAD_VOLT_LOW				3500
#define RAD_VOLT_HIGH				3700

typedef enum {
	BCDS_LF_B1_BT,
	BCDS_LF_B1_TT,
	BCDS_LF_B2_TT,
	BCDS_LF_B2_BT,
	BCDS_LF_B2_CHGN,
	BCDS_LF_B2_FAULTN,
	BCDS_LF_B1_CHGN,
	BCDS_LF_B1_FAULTN,
	BCDS_L2_ST,
	BCDS_L1_ST,
	BCDS_SPF_ST,
	BCDS_DUD,
	BCDS_L1_CHGN,
	BCDS_L1_FAULTN,
	BCDS_L2_CHGN,
	BCDS_L2_FAULTN
} bcds_conversions_t;

typedef enum {
	ADC_10B,
	ADC_12B,
	ADC_16B
} precise_t;

/**
 * Helpful regex: (\w*)_DATA -> $1_batch read_$1_batch(void);
 */
void read_lion_volts_batch(			lion_volts_batch batch);
void read_lion_current_batch(		lion_current_batch batch);
void read_lion_temps_batch(			lion_temps_batch batch);
void read_led_temps_batch(			led_temps_batch batch);
void read_lifepo_current_batch(		lifepo_current_batch batch);
void read_lifepo_volts_batch(		lifepo_volts_batch batch);
void read_lifepo_temps_batch(		lifepo_bank_temps_batch batch);
void read_ir_object_temps_batch(	ir_object_temps_batch batch);
void read_proc_temp_batch(			proc_temp_batch* batch);
void read_pdiode_batch(				pdiode_batch batch);
void read_ir_ambient_temps_batch(	ir_ambient_temps_batch batch);
void read_accel_batch(				accelerometer_batch accel_batch);
void read_gyro_batch(				gyro_batch gyr_batch);
void read_magnetometer_batch(		magnetometer_batch batch);
void read_led_current_batch(		led_current_batch batch);
void read_radio_temp_batch(			radio_temp_batch* batch);			// pointer to single value
void read_radio_volts_batch(		radio_volts_batch* batch);
//void read_radio_current_batch(		radio_current_batch* batch);
void read_bat_charge_volts_batch(	bat_charge_volts_batch batch);
void read_bat_charge_dig_sigs_batch(bat_charge_dig_sigs_batch* batch);	// pointer to single value
bool read_field_from_bcds(			bat_charge_dig_sigs_batch batch, bcds_conversions_t shift);
void read_imu_temp_batch(			imu_temp_batch* batch);				// pointer to single value
//void read_rail_5v_batch(			rail_5v_batch* batch);				// pointer to single value

#endif
