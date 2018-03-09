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
#define A_IR_OBJ_M					0
#define A_IR_OBJ_B					~0
#define A_IR_AMB_M					0
#define A_IR_AMB_B					~0
#define A_PD_M						0
#define A_PD_B						~0
#define A_LED_TEMP_M				0
#define A_LED_TEMP_B				~0
#define A_LED_CUR_REG_M				0
#define A_LED_CUR_REG_B				~0
#define A_LED_CUR_FLASH_M			0
#define A_LED_CUR_FLASH_B			~0
#define A_L_TEMP_M					0
#define A_L_TEMP_B					~0
#define A_LF_TEMP_M					0
#define A_LF_TEMP_B					~0
#define A_LF_CUR_REG_M				0
#define A_LF_CUR_REG_B				~0
#define A_LF_CUR_FLASH_M			0
#define A_LF_CUR_FLASH_B			~0
#define A_L_CUR_REG_M				0
#define A_L_CUR_REG_B				~0
#define A_L_CUR_HIGH_M				0
#define A_L_CUR_HIGH_B				~0
#define A_LF_VOLT_M					0
#define A_LF_VOLT_B					~0
#define A_L_VOLT_M					0
#define A_L_VOLT_B					~0
#define A_LREF_M					0
#define A_LREF_B					~0
#define A_PANELREF_M				0
#define A_PANELREF_B				~0
#define A_CHARGE_M					0
#define A_CHARGE_B					~0
#define A_GYRO_M					0
#define A_GYRO_B					~0
#define A_ACCEL_M					0
#define A_ACCEL_B					~0
#define A_MAG_M						0
#define A_MAG_B						~0
#define A_RAD_TEMP_M				0
#define A_RAD_TEMP_B				~0
#define A_IMU_TEMP_M				0
#define A_IMU_TEMP_B				~0
#define A_3V3_REF_M					0
#define A_3V3_REF_B					~0
#define A_3V6_REF_OFF_M				0
#define A_3V6_REF_OFF_B				~0
#define A_3V6_REF_ON_M				0
#define A_3V6_REF_ON_B				~0
#define A_3V6_SNS_OFF_M				0
#define A_3V6_SNS_OFF_B				~0
#define A_3V6_SNS_ON_M				0
#define A_3V6_SNS_ON_B				~0
#define A_5VREF_OFF_M				0
#define A_5VREF_OFF_B				~0
#define A_5VREF_ON_M				0
#define A_5VREF_ON_B				~0

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

typedef enum {
	S_IR_OBJ,
	S_IR_AMB,
	S_PD,
	S_LED_TEMP,
	S_LED_CUR_REG,
	S_LED_CUR_FLASH,
	S_L_TEMP,
	S_LF_TEMP,
	S_LF_CUR_REG,
	S_LF_CUR_FLASH,
	S_L_CUR_REG,
	S_L_CUR_HIGH,
	S_LF_VOLT,
	S_L_VOLT,
	S_LREF,
	S_PANELREF,
	S_CHARGE,
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
	S_5VREF_OFF,
	S_5VREF_ON
} sig_id_t;

uint get_low_bound_from_signal(sig_id_t sig);
uint get_high_bound_from_signal(sig_id_t sig);
int get_line_m_from_signal(sig_id_t sig);
int get_line_b_from_signal(sig_id_t sig);

#endif /* SENSOR_DEF_H_ */