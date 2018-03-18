/*
 * sensor_def.h
 *
 * Created: 3/8/18 8:46:13 PM
 *  Author: jleiken
 */ 


#ifndef SENSOR_DEF_H_
#define SENSOR_DEF_H_

#include <inttypes.h>

#define SNS_mA_TO_mV(mA) ((mA/2)+985)
#define IR_C_TO_RAW(temp_C) ((temp_C+273.15)/.02)

/************************************************************************/
/* POSSIBLE BOUNDS (for truncation)                                     */
/************************************************************************/
 // TODO: anything that has a ~0 (search file for ~0 and they should all be gone)
#define A_TEMP_M					(65/2)
#define A_TEMP_B					0
#define A_LED_SNS_M					650
#define A_LED_SNS_B					0
#define A_LF_SNS_M					(316/7)
#define A_LF_SNS_B					-960
#define A_LF_OSNS_M					(650/9)
#define A_LF_OSNS_B					0
#define A_LF_VOLT_M					(130/9)
#define A_LF_VOLT_B					0
#define A_L_SNS_M					20
#define A_L_SNS_B					150
#define A_L_VOLT_M					(130/9)
#define A_L_VOLT_B					0
#define A_LREF_M					(130/9)
#define A_LREF_B					0
#define A_PANELREF_M				(13/2)
#define A_PANELREF_B				0
#define A_IR_AMB_M					(63/8)
#define A_IR_AMB_B					-11657
#define A_GYRO_M					1
#define A_GYRO_B					32750
#define A_ACCEL_M					1
#define A_ACCEL_B					32768
#define A_MAG_M						(58/5)
#define A_MAG_B						2800
#define A_RAD_TEMP_M				(65/4)
#define A_RAD_TEMP_B				2000
#define A_IMU_TEMP_M				(14/9)
#define A_IMU_TEMP_B				20374

/************************************************************************/
/* ERROR BOUNDS                                                         */
/************************************************************************/
#define B_IR_OBJ_LOW					0		//note: not used (intentional)
#define B_IR_OBJ_HIGH					~0		 //note: not used (intentional)
#define B_IR_AMB_LOW					11657	//-40C
#define B_IR_AMB_HIGH					19908	//125C
#define B_PD_LOW						0		// note: not used (intentional)
#define B_PD_HIGH						~0		// note: not used (intentional)
#define B_TEMP_LOW						0
#define B_TEMP_HIGH						2000
#define B_LED_SNS_REG_LOW				0
#define B_LED_SNS_REG_HIGH				5		//167mA
#define B_LED_SNS_FLASH_LOW				20		//667mA
#define B_LED_SNS_FLASH_HIGH			100		//3.33A
#define B_LF_SNS_REG_LOW				960		//-1A
#define B_LF_SNS_REG_HIGH				1000	//1A	
#define B_LF_SNS_FLASH_LOW				1500	//26A
#define B_LF_SNS_FLASH_HIGH				2400	//65A
#define B_LF_OSNS_REG_LOW				0		//0A
#define B_LF_OSNS_REG_HIGH				25		//1.785A
#define B_LF_OSNS_FLASH_LOW				100		//7.143A
#define B_LF_OSNS_FLASH_HIGH			900		//~64A
#define B_LF_VOLT_LOW					0
#define B_LF_VOLT_HIGH					4000
/* lion sense */
#define B_L_SNS_OFF_LOW					985		//0mA
#define B_L_SNS_OFF_HIGH				1122	//275mA
#define B_L_SNS_IDLE_RAD_OFF_LOW		910		//-150mA
#define B_L_SNS_IDLE_RAD_OFF_HIGH		1085	//200mA
#define B_L_SNS_IDLE_RAD_ON_LOW			860		//-250mA
#define B_L_SNS_IDLE_RAD_ON_HIGH		1010	//-1970+
#define B_L_SNS_TRANSMIT_LOW			0		//-1970mA
#define B_L_SNS_TRANSMIT_HIGH			810		//-350mA
#define B_L_SNS_ANT_DEPLOY_LOW			0		//-1970mA
#define B_L_SNS_ANT_DEPLOY_HIGH			1122	//275mA
#define B_L_SNS_OFF_IDLE_TRANSITION_LOW		B_L_SNS_IDLE_RAD_OFF_LOW
#define B_L_SNS_OFF_IDLE_TRANSITION_HIGH	B_L_SNS_IDLE_RAD_ON_HIGH
#define B_L_SNS_IDLE_TRANS_TRANSITION_LOW	B_L_SNS_IDLE_RAD_ON_LOW
#define B_L_SNS_IDLE_TRANS_TRANSITION_HIGH	B_L_SNS_TRANSMIT_HIGH
/* end lion sense */
#define B_L_VOLT_LOW					0
#define B_L_VOLT_HIGH					4220
#define B_LREF_LOW						0
#define B_LREF_HIGH						4220
#define B_PANELREF_LOW					0
#define B_PANELREF_HIGH					10000
#define B_GYRO_LOW						0
#define B_GYRO_HIGH						~0
#define B_IMU_TEMP_LOW					0
#define B_IMU_TEMP_HIGH					~0
#define B_3V3_REF_LOW					1500 // 3000 mV
#define B_3V3_REF_HIGH					1800 // 3600 mV
#define B_3V6_REF_OFF_LOW				0
/* 3v6 ref */
#ifdef FLIGHT
	#define B_3V6_REF_OFF_HIGH				200
#else
	#define B_3V6_REF_OFF_HIGH				500
#endif
#define B_3V6_REF_ON_LOW				1700 // 3400 mV
#define B_3V6_REF_ON_HIGH				1850 // 3700 mV
#define B_3V6_SNS_OFF_LOW				0
/* 3v6 sense */
#ifdef FLIGHT
	#define B_3V6_SNS_OFF_HIGH				20
#else
	#define B_3V6_SNS_OFF_HIGH				90
#endif
#define B_3V6_SNS_ON_LOW				50
#ifdef FLIGHT
	#define B_3V6_SNS_ON_HIGH				200
#else
	#define B_3V6_SNS_ON_HIGH				400
#endif
#define B_3V6_SNS_TRANSMIT_LOW			600
#define B_3V6_SNS_TRANSMIT_HIGH			2000
#define B_3V6_SNS_OFF_IDLE_TRANSITION_LOW		B_3V6_SNS_OFF_LOW
#define B_3V6_SNS_OFF_IDLE_TRANSITION_HIGH		B_3V6_SNS_ON_HIGH
#define B_3V6_SNS_IDLE_TRANS_TRANSITION_LOW		B_3V6_SNS_ON_LOW
#define B_3V6_SNS_IDLE_TRANS_TRANSITION_HIGH	B_3V6_SNS_TRANSMIT_HIGH
/* 5v ref */
#define B_5VREF_OFF_LOW					0
#define B_5VREF_OFF_HIGH				120
#define B_5VREF_ON_LOW					1400 // 4733 mV
#define B_5VREF_ON_HIGH					1515 // 5122 mV
#define B_5VREF_TRANSITION_LOW			B_5VREF_OFF_LOW
#define B_5VREF_TRANSITION_HIGH			B_5VREF_ON_HIGH
#define B_RAD_TEMP_LOW					0
#define B_RAD_TEMP_HIGH					~0

typedef enum {
	S_IR_OBJ,
	S_IR_AMB,
	S_PD,
	S_LED_TEMP_REG,
	S_LED_TEMP_FLASH,
	S_LED_SNS,
	S_LED_SNS_REG,
	S_LED_SNS_FLASH,
	S_LED_SNS_FLASH_BATCH,
	S_LF_TEMP,
	S_LF_SNS_REG,
	S_LF_SNS_FLASH,
	S_LF_SNS_FLASH_BATCH,
	S_LF_OSNS_REG,
	S_LF_OSNS_FLASH,
	S_LF_OSNS_FLASH_BATCH,
	S_LF_VOLT,
	S_L_TEMP,
	S_L_SNS, // ONLY USE TO TRUNCATE, NOT LOG_IF_OUT_OF_BOUNDS
	S_L_SNS_OFF,
	S_L_SNS_IDLE_RAD_OFF,
	S_L_SNS_IDLE_RAD_ON,
	S_L_SNS_TRANSMIT,
	S_L_SNS_IDLE_TRANS_TRANSITION,
	S_L_SNS_OFF_IDLE_TRANSITION,
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
	S_3V6_SNS_OFF_IDLE_TRANSITION,
	S_3V6_SNS_IDLE_TRANS_TRANSITION,
	S_5VREF_ON,
	S_5VREF_OFF,
	S_5VREF_TRANSITION
} sig_id_t;

uint16_t get_low_bound_from_signal(sig_id_t sig);
uint16_t get_high_bound_from_signal(sig_id_t sig);
uint16_t get_line_m_from_signal(sig_id_t sig);
int16_t get_line_b_from_signal(sig_id_t sig);

#endif /* SENSOR_DEF_H_ */