/*
 * sensor_def.h
 *
 * Created: 3/8/18 8:46:13 PM
 *  Author: jleiken
 */ 


#ifndef SENSOR_DEF_H_
#define SENSOR_DEF_H_

#include "../global.h"

/************************************************************************/
/* POSSIBLE BOUNDS (for truncation)                                     */
/************************************************************************/
 // TODO: anything that has a ~0 (search file for ~0 and they should all be gone)
#define A_LED_TEMP_M				0
#define A_LED_TEMP_B				~0
#define A_LED_SNS_M					3/100
#define A_LED_SNS_B					0
#define A_LF_TEMP_M					0
#define A_LF_TEMP_B					~0
#define A_LF_SNS_M					197/4
#define A_LF_SNS_B					-960
#define A_LF_OSNS_M					500/7
#define A_LF_OSNS_B					0
#define A_LF_VOLT_M					130/9
#define A_LF_VOLT_B					0
#define A_L_TEMP_M					0
#define A_L_TEMP_B					~0
#define A_L_SNS_M					20
#define A_L_SNS_B					150
#define A_L_VOLT_M					130/9
#define A_L_VOLT_B					0
#define A_LREF_M					130/9
#define A_LREF_B					0
#define A_PANELREF_M				13/2
#define A_PANELREF_B				0
#define A_IR_AMB_M					63/8
#define A_IR_AMB_B					-11657
#define A_GYRO_M					1
#define A_GYRO_B					32750
#define A_ACCEL_M					1
#define A_ACCEL_B					32768
#define A_MAG_M						58/5
#define A_MAG_B						2800
#define A_RAD_TEMP_M				65/4
#define A_RAD_TEMP_B				2000
#define A_IMU_TEMP_M				14/9
#define A_IMU_TEMP_B				20374

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
#define B_LED_SNS_REG_LOW				0
#define B_LED_SNS_REG_HIGH				5
#define B_LED_SNS_FLASH_LOW				166
#define B_LED_SNS_FLASH_HIGH			2000
#define B_LF_TEMP_LOW					0
#define B_LF_TEMP_HIGH					~0
#define B_LF_SNS_REG_LOW				0
#define B_LF_SNS_REG_HIGH				~0
#define B_LF_SNS_FLASH_LOW				0
#define B_LF_SNS_FLASH_HIGH				~0
#define B_LF_OSNS_REG_LOW				0
#define B_LF_OSNS_REG_HIGH				~0
#define B_LF_OSNS_FLASH_LOW				0
#define B_LF_OSNS_FLASH_HIGH			~0
#define B_LF_VOLT_LOW					0
#define B_LF_VOLT_HIGH					4000
#define B_L_TEMP_LOW					0
#define B_L_TEMP_HIGH					~0
#define B_L_SNS_IDLE_RAD_OFF_LOW		10
#define B_L_SNS_IDLE_RAD_OFF_HIGH		150
#define B_L_SNS_IDLE_RAD_ON_LOW			100
#define B_L_SNS_IDLE_RAD_ON_HIGH		400
#define B_L_SNS_TRANSMIT_LOW			800
#define B_L_SNS_TRANSMIT_HIGH			1500
#define B_L_SNS_ANT_DEPLOY_LOW			200
#define B_L_SNS_ANT_DEPLOY_HIGH			3100
#define B_L_VOLT_LOW					0
#define B_L_VOLT_HIGH					4220
#define B_LREF_LOW						0
#define B_LREF_HIGH						4220
#define B_PANELREF_LOW					0
#define B_PANELREF_HIGH					9000
#define B_GYRO_LOW						0
#define B_GYRO_HIGH						~0
#define B_IMU_TEMP_LOW					0
#define B_IMU_TEMP_HIGH					~0
#define B_3V3_REF_LOW					3000
#define B_3V3_REF_HIGH					3600
#define B_3V6_REF_OFF_LOW				0
#define B_3V6_REF_OFF_HIGH				400
#define B_3V6_REF_ON_LOW				3400
#define B_3V6_REF_ON_HIGH				3700
#define B_3V6_SNS_OFF_LOW				0
#define B_3V6_SNS_OFF_HIGH				50
#define B_3V6_SNS_ON_LOW				50
#define B_3V6_SNS_ON_HIGH				120
#define B_3V6_SNS_TRANSMIT_LOW			700
#define B_3V6_SNS_TRANSMIT_HIGH			1100
#define B_5VREF_OFF_LOW					0
#define B_5VREF_OFF_HIGH				400
#define B_5VREF_ON_LOW					4800
#define B_5VREF_ON_HIGH					5100

typedef enum {
	S_IR_OBJ,
	S_IR_AMB,
	S_PD,
	S_LED_TEMP,
	S_LED_SNS,
	S_LED_SNS_REG,
	S_LED_SNS_FLASH,
	S_LF_TEMP,
	S_LF_SNS_REG,
	S_LF_SNS_FLASH,
	S_LF_OSNS_REG,
	S_LF_OSNS_FLASH,
	S_LF_VOLT,
	S_L_TEMP,
	S_L_SNS, // ONLY USE TO TRUNCATE, NOT LOG_IF_OUT_OF_BOUNDS
	S_L_SNS_IDLE_RAD_OFF,
	S_L_SNS_IDLE_RAD_ON,
	S_L_SNS_TRANSMIT,
	S_L_SNS_ANT_DEPLOY,
	S_L_VOLT,
	S_LREF,
	S_PANELREF,
	S_GYRO,
	S_ACCEL,
	S_MAG,
	S_RAD_TEMP,
	S_IMU_TEMP,
	S_3V3_REF,
	S_3V6_REF_OFF,
	S_3V6_REF_ON,
	S_3V6_SNS_OFF,
	S_3V6_SNS_ON,
	S_3V6_SNS_TRANSMIT,
	S_5VREF_ON,
	S_5VREF_OFF
} sig_id_t;

uint16_t get_low_bound_from_signal(sig_id_t sig);
uint16_t get_high_bound_from_signal(sig_id_t sig);
uint16_t get_line_m_from_signal(sig_id_t sig); // TODO: adjust the math and function definitions to reflect this
int16_t get_line_b_from_signal(sig_id_t sig);

#endif /* SENSOR_DEF_H_ */