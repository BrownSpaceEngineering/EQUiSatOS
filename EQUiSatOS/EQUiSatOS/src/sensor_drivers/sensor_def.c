/*
 * sensor_def.c
 *
 * Created: 3/8/18 8:49:14 PM
 *  Author: jleiken
 */ 

#include "sensor_def.h"

// IF SIG ISN'T IN THE CASES WE'RE CHECKING, THIS WILL RETURN ~0
uint16_t get_low_bound_from_signal(sig_id_t sig) {
	switch (sig) {
		case S_IR_OBJ:
			return B_IR_OBJ_LOW;
			break;
		case S_IR_AMB:
			return B_IR_AMB_LOW;
			break;
		case S_PD:
			return B_PD_LOW;
			break;
		case S_LED_TEMP:
			return B_LED_TEMP_LOW;
			break;
		case S_LED_SNS_REG:
			return B_LED_SNS_REG_LOW;
			break;
		case S_LED_SNS_FLASH:
			return B_LED_SNS_FLASH_LOW;
			break;
		case S_L_TEMP:
			return B_L_TEMP_LOW;
			break;
		case S_LF_TEMP:
			return B_LF_TEMP_LOW;
			break;
		case S_LF_SNS_REG:
			return B_LF_SNS_REG_LOW;
			break;
		case S_LF_SNS_FLASH:
			return B_LF_SNS_FLASH_LOW;
			break;
		case S_L_SNS_IDLE_RAD_OFF:
			return B_L_SNS_IDLE_RAD_OFF_LOW;
			break;
		case S_L_SNS_IDLE_RAD_ON:
			return B_L_SNS_IDLE_RAD_ON_LOW;
			break;
		case S_L_SNS_TRANSMIT:
			return B_L_SNS_TRANSMIT_LOW;
			break;
		case S_L_SNS_ANT_DEPLOY:
			return B_L_SNS_ANT_DEPLOY_LOW;
			break;
		case S_LF_VOLT:
			return B_LF_VOLT_LOW;
			break;
		case S_LF_OSNS_FLASH:
			return B_LF_OSNS_FLASH_LOW;
			break;
		case S_LF_OSNS_REG:
			return B_LF_OSNS_REG_LOW;
			break;
		case S_L_VOLT:
			return B_L_VOLT_LOW;
			break;
		case S_LREF:
			return B_LREF_LOW;
			break;
		case S_PANELREF:
			return B_PANELREF_LOW;
			break;
		case S_GYRO:
			return B_GYRO_LOW;
			break;
		case S_IMU_TEMP:
			return B_IMU_TEMP_LOW;
			break;
		case S_3V3_REF:
			return B_3V3_REF_LOW;
			break;
		case S_3V6_REF_OFF:
			return B_3V6_REF_OFF_LOW;
			break;
		case S_3V6_REF_ON:
			return B_3V6_REF_ON_LOW;
			break;
		case S_3V6_SNS_OFF:
			return B_3V6_SNS_OFF_LOW;
			break;
		case S_3V6_SNS_ON:
			return B_3V6_SNS_OFF_LOW;
			break;
		case S_3V6_SNS_TRANSMIT:
			return B_3V6_SNS_TRANSMIT_LOW;
			break;
		case S_5VREF_OFF:
			return B_5VREF_OFF_LOW;
			break;
		case S_5VREF_ON:
			return B_5VREF_ON_LOW;
			break;
		default:
			return ~0;
			break;
	}
}

// IF SIG ISN'T IN THE CASES WE'RE CHECKING, THIS WILL RETURN 0
uint16_t get_high_bound_from_signal(sig_id_t sig) {
	switch (sig) {
		case S_IR_OBJ:
			return B_IR_OBJ_HIGH;
			break;
		case S_IR_AMB:
			return B_IR_AMB_HIGH;
			break;
		case S_PD:
			return B_PD_HIGH;
			break;
		case S_LED_TEMP:
			return B_LED_TEMP_HIGH;
			break;
		case S_LED_SNS_REG:
			return B_LED_SNS_REG_HIGH;
			break;
		case S_LED_SNS_FLASH:
			return B_LED_SNS_FLASH_HIGH;
			break;
		case S_L_TEMP:
			return B_L_TEMP_HIGH;
			break;
		case S_LF_TEMP:
			return B_LF_TEMP_HIGH;
			break;
		case S_LF_SNS_REG:
			return B_LF_SNS_REG_HIGH;
			break;
		case S_LF_SNS_FLASH:
			return B_LF_SNS_FLASH_HIGH;
			break;
		case S_L_SNS_IDLE_RAD_OFF:
			return B_L_SNS_IDLE_RAD_OFF_HIGH;
			break;
		case S_L_SNS_IDLE_RAD_ON:
			return B_L_SNS_IDLE_RAD_ON_HIGH;
			break;
		case S_L_SNS_TRANSMIT:
			return B_L_SNS_TRANSMIT_HIGH;
			break;
		case S_L_SNS_ANT_DEPLOY:
			return B_L_SNS_ANT_DEPLOY_HIGH;
			break;
		case S_LF_VOLT:
			return B_LF_VOLT_HIGH;
			break;
		case S_LF_OSNS_FLASH:
			return B_LF_OSNS_FLASH_HIGH;
			break;
		case S_LF_OSNS_REG:
			return B_LF_OSNS_REG_HIGH;
			break;
		case S_L_VOLT:
			return B_L_VOLT_HIGH;
			break;
		case S_LREF:
			return B_LREF_HIGH;
			break;
		case S_PANELREF:
			return B_PANELREF_HIGH;
			break;
		case S_GYRO:
			return B_GYRO_HIGH;
			break;
		case S_IMU_TEMP:
			return B_IMU_TEMP_HIGH;
			break;
		case S_3V3_REF:
			return B_3V3_REF_HIGH;
			break;
		case S_3V6_REF_OFF:
			return B_3V6_REF_OFF_HIGH;
			break;
		case S_3V6_REF_ON:
			return B_3V6_REF_ON_HIGH;
			break;
		case S_3V6_SNS_OFF:
			return B_3V6_SNS_OFF_HIGH;
			break;
		case S_3V6_SNS_ON:
			return B_3V6_SNS_ON_HIGH;
			break;
		case S_3V6_SNS_TRANSMIT:
			return B_3V6_SNS_TRANSMIT_HIGH;
			break;
		case S_5VREF_OFF:
			return B_5VREF_OFF_HIGH;
			break;
		case S_5VREF_ON:
			return B_5VREF_ON_HIGH;
			break;
		default:
			return 0;
			break;
	}
}

// IF SIG ISN'T IN THE CASES WE'RE CHECKING, THIS WILL RETURN 0
uint16_t get_line_m_from_signal(sig_id_t sig) {
	switch (sig) {
		case S_IR_AMB:
			return A_IR_AMB_M;
			break;
		case S_LED_TEMP:
			return A_LED_TEMP_M;
			break;
		case S_LED_SNS:
			return A_LED_SNS_M;
			break;
		case S_LED_SNS_REG:
			return A_LED_SNS_M;
			break;
		case S_LED_SNS_FLASH:
			return A_LED_SNS_M;
			break;
		case S_L_TEMP:
			return A_L_TEMP_M;
			break;
		case S_LF_TEMP:
			return A_LF_TEMP_M;
			break;
		case S_LF_SNS_REG:
			return A_LF_SNS_M;
			break;
		case S_LF_SNS_FLASH:
			return A_LF_SNS_M;
			break;
		case S_L_SNS:
			return A_L_SNS_M;
			break;
		case S_L_SNS_IDLE_RAD_OFF:
			return A_L_SNS_M;
			break;
		case S_L_SNS_IDLE_RAD_ON:
			return A_L_SNS_M;
			break;
		case S_L_SNS_TRANSMIT:
			return A_L_SNS_M;
			break;
		case S_L_SNS_ANT_DEPLOY:
			return A_L_SNS_M;
			break;
		case S_LF_VOLT:
			return A_LF_VOLT_M;
			break;
		case S_LF_OSNS_FLASH:
			return A_LF_OSNS_M;
			break;
		case S_LF_OSNS_REG:
			return A_LF_OSNS_M;
			break;
		case S_L_VOLT:
			return A_L_VOLT_M;
			break;
		case S_LREF:
			return A_LREF_M;
			break;
		case S_PANELREF:
			return A_PANELREF_M;
			break;
		case S_GYRO:
			return A_GYRO_M;
			break;
		case S_ACCEL:
			return A_ACCEL_M;
			break;
		case S_MAG:
			return A_MAG_M;
			break;
		case S_RAD_TEMP:
			return A_RAD_TEMP_M;
			break;
		case S_IMU_TEMP:
			return A_IMU_TEMP_M;
			break;
		default:
			return 0; // so if the scaling value isn't found, the reading will ALWAYS BE 0
			break;
	}
}

// IF SIG ISN'T IN THE CASES WE'RE CHECKING, THIS WILL RETURN 0
int16_t get_line_b_from_signal(sig_id_t sig) {
	switch (sig) {
		case S_IR_AMB:
			return A_IR_AMB_B;
			break;
		case S_LED_TEMP:
			return A_LED_TEMP_B;
			break;
		case S_LED_SNS:
			return A_LED_SNS_B;
			break;
		case S_LED_SNS_REG:
			return A_LED_SNS_B;
			break;
		case S_LED_SNS_FLASH:
			return A_LED_SNS_B;
			break;
		case S_L_TEMP:
			return A_L_TEMP_B;
			break;
		case S_LF_TEMP:
			return A_LF_TEMP_B;
			break;
		case S_LF_SNS_REG:
			return A_LF_SNS_B;
			break;
		case S_LF_SNS_FLASH:
			return A_LF_SNS_B;
			break;
		case S_L_SNS:
			return A_L_SNS_B;
			break;
		case S_L_SNS_IDLE_RAD_OFF:
			return A_L_SNS_B;
			break;
		case S_L_SNS_IDLE_RAD_ON:
			return A_L_SNS_B;
			break;
		case S_L_SNS_TRANSMIT:
			return A_L_SNS_B;
			break;
		case S_L_SNS_ANT_DEPLOY:
			return A_L_SNS_B;
			break;
		case S_LF_VOLT:
			return A_LF_VOLT_B;
			break;
		case S_LF_OSNS_FLASH:
			return A_LF_OSNS_B;
			break;
		case S_LF_OSNS_REG:
			return A_LF_OSNS_B;
			break;
		case S_L_VOLT:
			return A_L_VOLT_B;
			break;
		case S_LREF:
			return A_LREF_B;
			break;
		case S_PANELREF:
			return A_PANELREF_B;
			break;
		case S_GYRO:
			return A_GYRO_B;
			break;
		case S_ACCEL:
			return A_ACCEL_B;
			break;
		case S_MAG:
			return A_MAG_B;
			break;
		case S_RAD_TEMP:
			return A_RAD_TEMP_B;
			break;
		case S_IMU_TEMP:
			return A_IMU_TEMP_B;
			break;
		default:
			return 0; // so if the scaling value isn't found, the reading will ALWAYS BE 0
			break;
	}
}