/*
 * sensor_def.c
 *
 * Created: 3/8/18 8:49:14 PM
 *  Author: jleiken
 */ 

#include "sensor_def.h"
#include "../errors.h"

// IF SIG ISN'T IN THE CASES WE'RE CHECKING, THIS WILL RETURN ~0
uint16_t get_low_bound_from_signal(sig_id_t sig) {
	switch (sig) {
		case S_IR_OBJ:
			return B_IR_OBJ_LOW;
		case S_IR_AMB:
			return B_IR_AMB_LOW;
		case S_PD:
			return B_PD_LOW;
		case S_LED_TEMP_REG:
			return B_TEMP_LOW;
		case S_LED_TEMP_FLASH:
			return B_TEMP_LOW;
		case S_LED_SNS_REG:
			return B_LED_SNS_REG_LOW;
		case S_LED_SNS_FLASH:
			return 0;  //intentional--logging errors based on a batch, not individual readings
		case S_LED_SNS_FLASH_BATCH:
			return B_LED_SNS_FLASH_LOW;
		case S_L_TEMP:
			return B_TEMP_LOW;
		case S_LF_TEMP:
			return B_TEMP_LOW;
		case S_LF_SNS_REG:
			return B_LF_SNS_REG_LOW;
		case S_LF_SNS_FLASH_BATCH:
			return B_LF_SNS_FLASH_LOW;
		case S_LF_SNS_FLASH:
			return 0; //intentional--logging errors based on a batch, not individual readings
		case S_L_SNS_OFF:
			return B_L_SNS_OFF_LOW;
		case S_L_SNS_IDLE_RAD_OFF:
			return B_L_SNS_IDLE_RAD_OFF_LOW;
		case S_L_SNS_IDLE_RAD_ON:
			return B_L_SNS_IDLE_RAD_ON_LOW;
		case S_L_SNS_TRANSMIT:
			return B_L_SNS_TRANSMIT_LOW;
		case S_L_SNS_OFF_IDLE_TRANSITION:
			return B_L_SNS_OFF_IDLE_TRANSITION_LOW;
		case S_L_SNS_IDLE_TRANS_TRANSITION:
			return B_L_SNS_IDLE_TRANS_TRANSITION_LOW;
		case S_L_SNS_ANT_DEPLOY:
			return B_L_SNS_ANT_DEPLOY_LOW;
		case S_LF_VOLT:
			return B_LF_VOLT_LOW;
		case S_LF_OSNS_FLASH_BATCH:
			return B_LF_OSNS_FLASH_LOW;
		case S_LF_OSNS_FLASH:
			return 0; //intentional--logging errors based on a batch
		case S_LF_OSNS_REG:
			return B_LF_OSNS_REG_LOW;
		case S_L_VOLT:
			return B_L_VOLT_LOW;
		case S_LREF:
			return B_LREF_LOW;
		case S_PANELREF:
			return B_PANELREF_LOW;
		case S_GYRO:
			return B_GYRO_LOW;
		case S_IMU_TEMP:
			return B_IMU_TEMP_LOW;
		case S_3V3_REF:
			return B_3V3_REF_LOW;
		case S_3V6_REF_OFF:
			return B_3V6_REF_OFF_LOW;
		case S_3V6_REF_ON:
			return B_3V6_REF_ON_LOW;
		case S_3V6_SNS_OFF:
			return B_3V6_SNS_OFF_LOW;
		case S_3V6_SNS_ON:
			return B_3V6_SNS_OFF_LOW;
		case S_3V6_SNS_TRANSMIT:
			return B_3V6_SNS_TRANSMIT_LOW;
		case S_3V6_SNS_OFF_IDLE_TRANSITION:
			return B_3V6_SNS_OFF_IDLE_TRANSITION_LOW;
		case S_3V6_SNS_IDLE_TRANS_TRANSITION:
			return B_3V6_SNS_IDLE_TRANS_TRANSITION_LOW;
		case S_5VREF_OFF:
			return B_5VREF_OFF_LOW;
		case S_5VREF_ON:
			return B_5VREF_ON_LOW;
		case S_5VREF_TRANSITION:
			return B_5VREF_TRANSITION_LOW;
		case S_RAD_TEMP:
			return B_RAD_TEMP_LOW;
		default:
			return ~0;
	}
}

// IF SIG ISN'T IN THE CASES WE'RE CHECKING, THIS WILL RETURN 0
uint16_t get_high_bound_from_signal(sig_id_t sig) {
	switch (sig) {
		case S_IR_OBJ:
			return B_IR_OBJ_HIGH;
		case S_IR_AMB:
			return B_IR_AMB_HIGH;
		case S_PD:
			return B_PD_HIGH;
		case S_LED_TEMP_REG:
			return B_TEMP_HIGH;
		case S_LED_TEMP_FLASH:
			return B_TEMP_HIGH;
		case S_LED_SNS_REG:
			return B_LED_SNS_REG_HIGH;
		case S_LED_SNS_FLASH:
			return ~0;  //intentional--logging errors based on a batch
		case S_LED_SNS_FLASH_BATCH:
			return B_LED_SNS_FLASH_HIGH;
		case S_L_TEMP:
			return B_TEMP_HIGH;
		case S_LF_TEMP:
			return B_TEMP_HIGH;
		case S_LF_SNS_REG:
			return B_LF_SNS_REG_HIGH;
		case S_LF_SNS_FLASH_BATCH:
			return B_LF_SNS_FLASH_HIGH;
		case S_LF_SNS_FLASH:
			return ~0; //intentional--logging errors based on a batch
		case S_L_SNS_OFF:
			return B_L_SNS_OFF_HIGH;
		case S_L_SNS_IDLE_RAD_OFF:
			return B_L_SNS_IDLE_RAD_OFF_HIGH;
		case S_L_SNS_IDLE_RAD_ON:
			return B_L_SNS_IDLE_RAD_ON_HIGH;
		case S_L_SNS_TRANSMIT:
			return B_L_SNS_TRANSMIT_HIGH;
		case S_L_SNS_OFF_IDLE_TRANSITION:
			return B_L_SNS_OFF_IDLE_TRANSITION_HIGH;
		case S_L_SNS_IDLE_TRANS_TRANSITION:
			return B_L_SNS_IDLE_TRANS_TRANSITION_HIGH;
		case S_L_SNS_ANT_DEPLOY:
			return B_L_SNS_ANT_DEPLOY_HIGH;
		case S_LF_VOLT:
			return B_LF_VOLT_HIGH;		
		case S_LF_OSNS_FLASH_BATCH:
			return B_LF_OSNS_FLASH_HIGH;
		case S_LF_OSNS_FLASH:
			return ~0; //intentional--logging errors based on a batch;
		case S_LF_OSNS_REG:
			return B_LF_OSNS_REG_HIGH;
		case S_L_VOLT:
			return B_L_VOLT_HIGH;
		case S_LREF:
			return B_LREF_HIGH;
		case S_PANELREF:
			return B_PANELREF_HIGH;
		case S_GYRO:
			return B_GYRO_HIGH;
		case S_IMU_TEMP:
			return B_IMU_TEMP_HIGH;
		case S_3V3_REF:
			return B_3V3_REF_HIGH;
		case S_3V6_REF_OFF:
			return B_3V6_REF_OFF_HIGH;
		case S_3V6_REF_ON:
			return B_3V6_REF_ON_HIGH;
		case S_3V6_SNS_OFF:
			return B_3V6_SNS_OFF_HIGH;
		case S_3V6_SNS_ON:
			return B_3V6_SNS_ON_HIGH;
		case S_3V6_SNS_TRANSMIT:
			return B_3V6_SNS_TRANSMIT_HIGH;
		case S_3V6_SNS_OFF_IDLE_TRANSITION:
			return B_3V6_SNS_OFF_IDLE_TRANSITION_HIGH;
		case S_3V6_SNS_IDLE_TRANS_TRANSITION:
			return B_3V6_SNS_IDLE_TRANS_TRANSITION_HIGH;
		case S_5VREF_OFF:
			return B_5VREF_OFF_HIGH;
		case S_5VREF_ON:
			return B_5VREF_ON_HIGH;
		case S_5VREF_TRANSITION:
			return B_5VREF_TRANSITION_HIGH;
		case S_RAD_TEMP:
			return B_RAD_TEMP_HIGH;
		default:
			return 0;
	}
}

// IF SIG ISN'T IN THE CASES WE'RE CHECKING, THIS WILL RETURN 0
uint16_t get_line_m_from_signal(sig_id_t sig) {
	switch (sig) {
		case S_IR_AMB:
			return A_IR_AMB_M;
		case S_LED_TEMP_REG:
			return A_TEMP_M;
		case S_LED_TEMP_FLASH:
			return A_TEMP_M;
		case S_LED_SNS:
			return A_LED_SNS_M;
		case S_LED_SNS_REG:
			return A_LED_SNS_M;
		case S_LED_SNS_FLASH:
			return A_LED_SNS_M;
		case S_LED_SNS_FLASH_BATCH:
			return A_LED_SNS_M;
		case S_L_TEMP:
			return A_TEMP_M;
		case S_LF_TEMP:
			return A_TEMP_M;
		case S_LF_SNS_REG:
			return A_LF_SNS_M;
		case S_LF_SNS_FLASH:
			return A_LF_SNS_M;
		case S_LF_SNS_FLASH_BATCH:
		return A_LF_SNS_M;
		case S_L_SNS:
			return A_L_SNS_M;
		case S_L_SNS_IDLE_RAD_OFF:
			return A_L_SNS_M;
		case S_L_SNS_IDLE_RAD_ON:
			return A_L_SNS_M;
		case S_L_SNS_TRANSMIT:
			return A_L_SNS_M;
		case S_L_SNS_ANT_DEPLOY:
			return A_L_SNS_M;
		case S_LF_VOLT:
			return A_LF_VOLT_M;
		case S_LF_OSNS_FLASH:
			return A_LF_OSNS_M;
		case S_LF_OSNS_FLASH_BATCH:
		return A_LF_OSNS_M;
		case S_LF_OSNS_REG:
			return A_LF_OSNS_M;
		case S_L_VOLT:
			return A_L_VOLT_M;
		case S_LREF:
			return A_LREF_M;
		case S_PANELREF:
			return A_PANELREF_M;
		case S_GYRO:
			return A_GYRO_M;
		case S_ACCEL:
			return A_ACCEL_M;
		case S_MAG:
			return A_MAG_M;
		case S_RAD_TEMP:
			return A_RAD_TEMP_M;
		case S_IMU_TEMP:
			return A_IMU_TEMP_M;
		default:
			log_error(ELOC_SCALING_M, ECODE_UNEXPECTED_CASE, false);
			return 0; // so if the scaling value isn't found, the reading will ALWAYS BE 0
	}
}

// IF SIG ISN'T IN THE CASES WE'RE CHECKING, THIS WILL RETURN 0
int16_t get_line_b_from_signal(sig_id_t sig) {
	switch (sig) {
		case S_IR_AMB:
			return A_IR_AMB_B;
		case S_LED_TEMP_REG:
			return A_TEMP_B;
		case S_LED_TEMP_FLASH:
			return A_TEMP_B;
		case S_LED_SNS:
			return A_LED_SNS_B;
		case S_LED_SNS_REG:
			return A_LED_SNS_B;
		case S_LED_SNS_FLASH:
			return A_LED_SNS_B;
		case S_LED_SNS_FLASH_BATCH:
			return A_LED_SNS_B;			
		case S_L_TEMP:
			return A_TEMP_B;
		case S_LF_TEMP:
			return A_TEMP_B;
		case S_LF_SNS_REG:
			return A_LF_SNS_B;
		case S_LF_SNS_FLASH:
			return A_LF_SNS_B;
		case S_LF_SNS_FLASH_BATCH:
			return A_LF_SNS_B;
		case S_L_SNS:
			return A_L_SNS_B;
		case S_L_SNS_IDLE_RAD_OFF:
			return A_L_SNS_B;
		case S_L_SNS_IDLE_RAD_ON:
			return A_L_SNS_B;
		case S_L_SNS_TRANSMIT:
			return A_L_SNS_B;
		case S_L_SNS_ANT_DEPLOY:
			return A_L_SNS_B;
		case S_LF_VOLT:
			return A_LF_VOLT_B;
		case S_LF_OSNS_FLASH:
			return A_LF_OSNS_B;
		case S_LF_OSNS_FLASH_BATCH:
		return A_LF_OSNS_B;
		case S_LF_OSNS_REG:
			return A_LF_OSNS_B;
		case S_L_VOLT:
			return A_L_VOLT_B;
		case S_LREF:
			return A_LREF_B;
		case S_PANELREF:
			return A_PANELREF_B;
		case S_GYRO:
			return A_GYRO_B;
		case S_ACCEL:
			return A_ACCEL_B;
		case S_MAG:
			return A_MAG_B;
		case S_RAD_TEMP:
			return A_RAD_TEMP_B;
		case S_IMU_TEMP:
			return A_IMU_TEMP_B;
		default:
			log_error(ELOC_SCALING_B, ECODE_UNEXPECTED_CASE, false);
			return 0; // so if the scaling value isn't found, the reading will ALWAYS BE 0
	}
}