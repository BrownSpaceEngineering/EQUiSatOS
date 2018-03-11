/*
 * sensor_read_commands.h
 *
 * This file essentially serves as an interface between sensor drivers and RTOS.
 * Sensor drivers are (optimistically) standalone, while this file adds complexity
 * such as time delays and mutexes.
 *
 * Created: 11/1/2016 8:17:44 PM
 *  Author: mckenna
 */

#ifndef SENSOR_READ_COMMANDS_H
#define	SENSOR_READ_COMMANDS_H

#include "../global.h"
#include "../testing_functions/equisim_simulated_data.h"
#include "sensor_def.h"

// TODO: all of these
/************************************************************************/
/* CRITICAL HARDWARE TIMINGS                                            */
/************************************************************************/
<<<<<<< HEAD
#define EN_5V_POWER_UP_DELAY_MS			10  // note we hold a mutex for this time
#define IR_WAKE_DELAY					300

/* PDIODE BOUNDS */
#define PDIODE_00_01					460
#define PDIODE_01_10					505
#define PDIODE_10_11					550

/************************************************************************/
/* ERROR BOUNDS                                                         */
/************************************************************************/
#define B_IR_OBJ_LOW					0
#define B_IR_OBJ_HIGH					~0
#define B_IR_AMB_LOW					0
#define B_IR_AMB_HIGH					~0
#define B_PD_LOW						0
#define B_PD_HIGH						~0
#define B_LED_TEMP_LOW					0
#define B_LED_TEMP_HIGH					~0
#define B_LED_CUR_REG_LOW				0
#define B_LED_CUR_REG_HIGH				~0
#define B_LED_CUR_FLASH_LOW				0
#define B_LED_CUR_FLASH_HIGH			~0
#define B_L_TEMP_LOW					0
#define B_L_TEMP_HIGH					~0
#define B_LF_TEMP_LOW					0
#define B_LF_TEMP_HIGH					~0
#define B_LF_CUR_REG_LOW				0
#define B_LF_CUR_REG_HIGH				~0
#define B_LF_CUR_FLASH_LOW				0
#define B_LF_CUR_FLASH_HIGH				~0
#define B_L_CUR_REG_LOW					0
#define B_L_CUR_REG_HIGH				~0
#define B_L_CUR_HIGH_LOW				0
#define B_L_CUR_HIGH_HIGH				~0
#define B_LF_VOLT_LOW					0
#define B_LF_VOLT_HIGH					~0
#define B_L_VOLT_LOW					0
#define B_L_VOLT_HIGH					~0
#define B_LREF_LOW						0
#define B_LREF_HIGH						~0
#define B_PANELREF_LOW					0
#define B_PANELREF_HIGH					~0
#define B_CHARGE_LOW					0
#define B_CHARGE_HIGH					~0
#define B_GYRO_LOW						0
#define B_GYRO_HIGH						~0
#define B_IMU_TEMP_LOW					0
#define B_IMU_TEMP_HIGH					~0
#define B_3V3_REF_LOW					3000
#define B_3V3_REF_HIGH					3600
#define B_3V6_REF_OFF_LOW				0
#define B_3V6_REF_OFF_HIGH				400
#define B_3V6_REF_ON_LOW				3400
#define B_3V6_REF_ON_HIGH				3800
#define B_3V6_SNS_OFF_LOW				0
#define B_3V6_SNS_OFF_HIGH				10
#define B_3V6_SNS_ON_LOW				50
#define B_3V6_SNS_ON_HIGH				2000
#define B_5VREF_OFF_LOW					0
#define B_5VREF_OFF_HIGH				400
#define B_5VREF_ON_LOW					4800
#define B_5VREF_ON_HIGH					5200
=======
#define EN_5V_POWER_UP_DELAY_MS		10  // note we hold a mutex for this time
#define IR_WAKE_DELAY				300 // TODO: add unit
>>>>>>> 84830eb97f621eb7cf559d57acda75d370265015

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

/************************************************************************/
/* HARDWARE MUTEXs - see https://www.draw.io/#G1bt9XDgZvyObssMtjbUi8nUNwu0kcQpVI */
/* **ONLY** can be used outside this task in the flash_activate_task (for speed purposes) */
/************************************************************************/
#define HARDWARE_MUTEX_WAIT_TIME_TICKS	(1000 / portTICK_PERIOD_MS)
StaticSemaphore_t _i2c_mutex_d;
SemaphoreHandle_t i2c_mutex;
StaticSemaphore_t _irpow_mutex_d;
SemaphoreHandle_t irpow_mutex;
StaticSemaphore_t _processor_adc_mutex_d;
SemaphoreHandle_t processor_adc_mutex;

/************************************************************************/
/* FUNCTIONS                                                            */
/************************************************************************/
void init_sensor_read_commands(void);

/* primary */
void read_lion_volts_batch(			lion_volts_batch batch);
void read_ad7991_batbrd(			lion_current_batch batch1, panelref_lref_batch batch2);
void en_and_read_lion_temps_batch(	lion_temps_batch batch);
void read_lifepo_volts_batch(		lifepo_volts_batch batch);
void read_ir_object_temps_batch(	ir_object_temps_batch batch);
void read_imu_temp_batch(			imu_temp_batch* batch); // pointer to single value
void read_pdiode_batch(				pdiode_batch* batch); // pointer to single value
void read_ir_ambient_temps_batch(	ir_ambient_temps_batch batch);
void read_accel_batch(				accelerometer_batch accel_batch);
void read_gyro_batch(				gyro_batch gyr_batch);
void read_magnetometer_batch(		magnetometer_batch batch);
void read_radio_temp_batch(			radio_temp_batch* batch);			// pointer to single value
void read_ad7991_ctrlbrd(			ad7991_ctrlbrd_batch batch); //TODO: UNCOMMENT IF YOU NEED THIS; otherwise it goes
void read_bat_charge_dig_sigs_batch(bat_charge_dig_sigs_batch* batch);	// pointer to single value
bool read_field_from_bcds(			bat_charge_dig_sigs_batch batch, bcds_conversions_t shift);
void read_lifepo_current_batch(		lifepo_current_batch batch, bool flashing_now);

/* non-thread safe functions that should ONLY be called from FLASH TASK */
void _read_led_temps_batch_unsafe(			led_temps_batch batch);
void _read_lifepo_temps_batch_unsafe(		lifepo_bank_temps_batch batch);
void _read_lifepo_current_batch_unsafe(		lifepo_current_batch batch, bool flashing_now);
void _read_lifepo_volts_batch_unsafe(		lifepo_volts_batch batch);
void _read_led_current_batch_unsafe(		led_current_batch batch, bool flashing_now);
void _read_gyro_batch_unsafe(				gyro_batch gyr_batch);

/* utility */
void read_lion_volts_precise(uint16_t* val_1, uint16_t* val_2);
void read_lifepo_volts_precise(uint16_t* val_1, uint16_t* val_2, uint16_t* val_3, uint16_t* val_4);
void read_lion_current_precise(uint16_t* val_1, uint16_t* val_2);
void read_lifepo_current_precise(uint16_t* val_1, uint16_t* val_2, uint16_t* val_3, uint16_t* val_4);

void _enable_ir_pow_if_necessary(void);
bool _set_5v_enable(bool on);
void verify_regulators(void);
void verify_flash_readings(bool flashing);

#endif
