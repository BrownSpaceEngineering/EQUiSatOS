/*
 * global.h
 *
 * Created: 4/12/2017 9:35:47 PM
 *  Author: jleiken
 */ 


#ifndef GLOBAL_H_
#define GLOBAL_H_

#include <asf.h>

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

#define ECODE_IR_1			1
#define ECODE_IR_2			2
#define ECODE_IR_3			3
#define ECODE_IR_4			4
#define ECODE_IR_5			5
#define ECODE_IR_6			6

#define ECODE_PD_1			7
#define ECODE_PD_2			8
#define ECODE_PD_3			9
#define ECODE_PD_4			10
#define ECODE_PD_5			11
#define ECODE_PD_6			12

#define ECODE_TEMP_1		13
#define ECODE_TEMP_2		14
#define ECODE_TEMP_3		15
#define ECODE_TEMP_4		16
#define ECODE_TEMP_5		17
#define ECODE_TEMP_6		18
#define ECODE_TEMP_7		19
#define ECODE_TEMP_8		20

#define ECODE_RADIO_TEMP	21

#define ECODE_IMU_ACC		22
#define ECODE_IMU_GYRO		23
#define ECODE_IMU_MAG		24

#define ECODE_LED1SNS		25
#define ECODE_LED2SNS		26
#define ECODE_LED3SNS		27
#define ECODE_LED4SNS		28
#define ECODE_LFB1OSNS		29
#define ECODE_LFB1SNS		30
#define ECODE_LFB2OSNS		31
#define ECODE_LFB2SNS		32
#define ECODE_LF1REF		33
#define ECODE_LF2REF		34
#define ECODE_LF3REF		35
#define ECODE_LF4REF		36
#define ECODE_L1_REF		37
#define ECODE_L2_REF		38
#define ECODE_DET_RTN		39


#endif /* GLOBAL_H_ */