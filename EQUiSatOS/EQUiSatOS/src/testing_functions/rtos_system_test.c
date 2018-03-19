/*
 * rtos_system_test.c
 *
 * Created: 2/11/2018 18:30:09
 *  Author: mcken
 */ 
#include "rtos_system_test.h"

#ifdef RTOS_SYSTEM_TEST_ONLY_NEW
	const bool only_print_recent_data = true;
#else
	const bool only_print_recent_data = true;
#endif

/************************************************************************/
/* Sensor type printing methods                                         */
/************************************************************************/
void print_accel_batch(accelerometer_batch batch) {
	print("accel\n");
	print("\tx: %d\t%d\n", batch[0], (int16_t) untruncate(batch[0], S_ACCEL));
	print("\ty: %d\t%d\n", batch[1], (int16_t) untruncate(batch[1], S_ACCEL));
	print("\tz: %d\t%d\n", batch[2], (int16_t) untruncate(batch[2], S_ACCEL));
}
void print_gyro_batch(gyro_batch batch) {
	print("gyro\n");
	print("\tx: %d\t%d\n", batch[0], (int16_t) untruncate(batch[0], S_GYRO));
	print("\ty: %d\t%d\n", batch[1], (int16_t) untruncate(batch[1], S_GYRO));
	print("\tz: %d\t%d\n", batch[2], (int16_t) untruncate(batch[2], S_GYRO));
}
void print_magnetometer_batch(magnetometer_batch batch) {
	print("mag\n");
	print("\tx: %d\t%d\n", batch[0], (int16_t) untruncate(batch[0], S_MAG));
	print("\ty: %d\t%d\n", batch[1], (int16_t) untruncate(batch[1], S_MAG));
	print("\tz: %d\t%d\n", batch[2], (int16_t) untruncate(batch[2], S_MAG));
}
void print_ir_ambient_temps_batch(ir_ambient_temps_batch batch) {
	print("ir ambs\n");
	for (int i = 0; i < 6; i++) {
		print("\t%d: %d\t%d C\n", i+1, untruncate(batch[i], S_IR_AMB), (int16_t) dataToTemp(untruncate(batch[i], S_IR_AMB)));
	}
}

static int16_t ad590_to_temp(uint16_t temp_mV)  {
	//float current = ((float)mV)/1000/2197. -0.000153704; //converts from V to A
	//float tempInC = (current)*1000000-273;// T = 454*V in C
	float tempInC = ((float) temp_mV) *0.1286 - 107.405;
	return (int16_t) tempInC;
}
void print_ir_object_temps_batch(ir_object_temps_batch batch) {
	print("ir objs\n");
	for (int i = 0; i < 6; i++) {
		print("\t%d: %d\t%d C\n", i+1, batch[i]<<8, (int16_t) dataToTemp(batch[i]));
	}
}
void print_lion_volts_batch(lion_volts_batch batch) {
	print("L1_REF: %d %d mV\n", batch[0], untruncate(batch[0], S_L_VOLT));
	print("L2_REF: %d %d mV\n", batch[1], untruncate(batch[1], S_L_VOLT));
}
static void print_lion_current_batch(lion_current_batch batch) {
	print("L1_SNS: %d %d mV\t%d mA\n", batch[0], untruncate(batch[0], S_L_SNS), ((int16_t)untruncate(batch[0], S_L_SNS)-985)*2);
	print("L2_SNS: %d %d mV\t%d mA\n", batch[1], untruncate(batch[1], S_L_SNS), ((int16_t)untruncate(batch[1], S_L_SNS)-985)*2);
}
void print_lion_temps_batch(lion_temps_batch batch) {
	print("L1_TEMP: %d\t%d mV\t%d C\n", batch[0], untruncate(batch[0], S_L_TEMP), ad590_to_temp(untruncate(batch[0], S_L_TEMP)));
	print("L2_TEMP: %d\t%d mV\t %d C\n", batch[1], untruncate(batch[1], S_L_TEMP), ad590_to_temp(untruncate(batch[1], S_L_TEMP)));
}
void print_led_temps_batch(led_temps_batch batch) {
	print("LED Temps\n");
	for (int i = 0; i < 4; i++) {
		print("\t%d: %d\t%d mV\t%dC\n", i+1, batch[i], untruncate(batch[i], S_LED_TEMP_REG), ad590_to_temp(untruncate(batch[i], S_LED_TEMP_REG)));
	}
}
void print_lifepo_temps_batch(lifepo_bank_temps_batch batch) {
	print("lifepo bat temps\n");
	print("\t1: %d\t%d mV\t%d C\n", batch[0], untruncate(batch[0], S_LF_TEMP), ad590_to_temp(untruncate(batch[0], S_LF_TEMP)));
	print("\t2: %d\t%d mV\t%d C\n", batch[1], untruncate(batch[0], S_LF_TEMP), ad590_to_temp(untruncate(batch[1], S_LF_TEMP)));
}
void print_lifepo_volts_batch(lifepo_volts_batch batch) {
	print("lifepo volts\n");
	for (int i = 0; i < 4; i++) {
		print("\tLF%dREF: %d\t%d mV\n", i+1, batch[i], untruncate(batch[i], S_LF_VOLT));
	}
}
void print_lifepo_current_batch(lifepo_current_batch batch) {	
	print("\tLFB1SNS: %d\t%d mV\t%d mA\n", batch[0], untruncate(batch[0], S_LF_SNS_REG), ((int16_t) untruncate(batch[0], S_LF_SNS_REG)-980)*50);
	print("\tLFB1OSNS: %d\t%d mV\t%d mA\n", batch[1], untruncate(batch[1], S_LF_OSNS_REG), untruncate(batch[1], S_LF_OSNS_REG)*7143/100);
	print("\tLFB2SNS: %d\t%d mV\t%d mA\n", batch[2], untruncate(batch[2], S_LF_SNS_REG), ((int16_t) untruncate(batch[2], S_LF_SNS_REG)-979)*50);
	print("\tLFB2OSNS: %d\t%d mV\t%d mA\n\n", batch[3], untruncate(batch[3], S_LF_OSNS_REG), untruncate(batch[3], S_LF_OSNS_REG)*7143/100);
}
void print_led_current_batch(led_current_batch batch) {
	print("led current\n");
	for (int i = 0; i < 4; i++) {
		print("\tLED%dSNS: %d\t%d mV\t%d mA\n", i+1, batch[i], untruncate(batch[i], S_LED_SNS), untruncate(batch[i], S_LED_SNS)*100/3);
	}
}
void print_panelref_lref_batch(panelref_lref_batch batch) {
	print("refs: PANELREF: %d %d mV\t%d mV\nL_REF: %d %d mV\t%d mV\n", batch[0], untruncate(batch[0], S_PANELREF), (untruncate(batch[0], S_PANELREF)-130)*558/100, batch[1], untruncate(batch[1], S_LREF), (untruncate(batch[1], S_LREF)-50)*2717/1000);
}
void print_radio_temp_batch(radio_temp_batch batch) {
	print("radio temp: %d\t: %d C\n", batch, untruncate(batch, S_RAD_TEMP)/10);
}
void print_imu_temp_batch(imu_temp_batch batch) {
	print("imu temp: %d\tBAD: %d C\n", batch, untruncate(batch, S_IMU_TEMP));
}
void print_satellite_state_history_batch(satellite_history_batch batch) {
	print("---Satellite History Batch---\n");
	print("antenna deployed:   %d\n", batch.antenna_deployed);
	print("lion 1 charged:     %d\n", batch.lion_1_charged);
	print("lion 2 charged:     %d\n", batch.lion_2_charged);
	print("lifepo b1 charged:  %d\n", batch.lifepo_b1_charged);
	print("lifepo b2 charged:  %d\n", batch.lifepo_b2_charged);
	print("first flash:        %d\n", batch.lifepo_b2_charged);
	print("prog_mem_rewritten: %d\n", batch.prog_mem_rewritten);
}
void print_bat_charge_dig_sigs_batch(bat_charge_dig_sigs_batch batch) {
	print("---Battery Charging Digital Signals---\n");
	print("L1_RUN_CHG:     %d\n", batch & 0x1);
	print("L2_RUN_CHG:     %d\n", (batch >> 1) & 0x1);
	print("LF_B1_RUN_CHG:  %d\n", (batch >> 2) & 0x1);
	print("LF_B2_RUN_CHG:  %d\n", (batch >> 3) & 0x1);
	print("LF_B2_CHGN:     %d\n", (batch >> 4) & 0x1);
	print("LF_B2_FAULTN:   %d\n", (batch >> 5) & 0x1);
	print("LF_B1_FAULTN:   %d\n", (batch >> 6) & 0x1);
	print("LF_B1_CHGN:     %d\n", (batch >> 7) & 0x1);
	print("L2_ST:          %d\n", (batch >> 8) & 0x1);
	print("L1_ST:          %d\n", (batch >> 9) & 0x1);
	print("L1_DISG:        %d\n", (batch >> 10) & 0x1);
	print("L2_DISG:        %d\n", (batch >> 11) & 0x1);
	print("L1_CHGN:        %d\n", (batch >> 12) & 0x1);
	print("L1_FAULTN:      %d\n", (batch >> 13) & 0x1);
	print("L2_CHGN:        %d\n", (batch >> 14) & 0x1);
	print("L2_FAULTN:      %d\n", (batch >> 15) & 0x1);
}
void print_pdiode_batch(pdiode_batch batch) {
	for (int i = 0; i < 6; i++){
		print("pdiode %d: %d\n",i, (uint16_t)(batch>>(i*2))&0b11);
	}
}

/************************************************************************/
/* Stack type element-printing methods                                  */
/************************************************************************/
static void print_stack_type_header(const char* header, int i, uint32_t timestamp, bool transmitted) {
	print("%2d: ---------%s---------\n", i, header);
	print("timestamp: %d \t %s\n", timestamp, transmitted ? "TRANSMITTED" : "not transmitted");
}

void print_idle_data(idle_data_t* data, int i) {
	print_stack_type_header("Idle Data Packet", i, data->timestamp, data->transmitted);
	print_satellite_state_history_batch(data->satellite_history);
	print_lion_volts_batch(data->lion_volts_data);
	print_lion_current_batch(data->lion_current_data);
	print_lion_temps_batch(data->lion_temps_data);
	print_panelref_lref_batch(data->panelref_lref_data);
	print_bat_charge_dig_sigs_batch(data->bat_charge_dig_sigs_data);
	print_radio_temp_batch(data->radio_temp_data);
	print_imu_temp_batch(data->imu_temp_data);
	print_ir_ambient_temps_batch(data->ir_amb_temps_data);
}

void print_attitude_data(attitude_data_t* data, int i) {
	print_stack_type_header("Attitude Data Packet", i, data->timestamp, data->transmitted);
	print_ir_object_temps_batch(data->ir_obj_temps_data);
	print_pdiode_batch(data->pdiode_data);
	print("accel batches (500ms apart):\n");
	print_accel_batch(data->accelerometer_data[0]);
	print_accel_batch(data->accelerometer_data[1]);
	print("magnetometer batches (500ms apart):\n");
	print_magnetometer_batch(data->magnetometer_data[0]);
	print_magnetometer_batch(data->magnetometer_data[1]);
	print_gyro_batch(data->gyro_data);
}

void print_flash_data(flash_data_t* data, int i_global) {
	print("---------LED BURST Data---------\n");
	print_stack_type_header("Flash Data Packet", i_global, data->timestamp, data->transmitted);
	//print("---LED Temp Burst Data---\n");
	for (int i = 0; i < FLASH_DATA_ARR_LEN; i++) {
		print_led_temps_batch(data->led_temps_data[i]);
	}
	//print("---LiFePo Temp Burst Data---\n");
	for (int i = 0; i < FLASH_DATA_ARR_LEN; i++) {
		print_lifepo_temps_batch(data->lifepo_bank_temps_data[i]);
	}
	//print("---LiFePo Current Burst Data---\n");
	for (int i = 0; i < FLASH_DATA_ARR_LEN; i++) {
		print_lifepo_current_batch(data->lifepo_current_data[i]);
	}
	//print("---LiFePo Volts Burst Data---\n");
	for (int i = 0; i < FLASH_DATA_ARR_LEN; i++) {
		print_lifepo_volts_batch(data->lifepo_volts_data[i]);
	}
	//print("---Led Current Burst Data---\n");
	for (int i = 0; i < FLASH_DATA_ARR_LEN; i++) {
		print_led_current_batch(data->led_current_data[i]);
	}
	//print("---Gyro Burst Data---\n");
	for (int i = 0; i < FLASH_DATA_ARR_LEN; i++) {
		print_gyro_batch(data->gyro_data[i]);
	}
	print("---------END LED BURST Data---------\n");
}

void print_flash_cmp_data(flash_cmp_data_t* data, int i) {
	print("---------END LED COMPARISON Data---------\n");
	print_stack_type_header("Flash Compare Data Packet", i, data->timestamp, data->transmitted);
	print_led_temps_batch(data->led_temps_avg_data);
	print_lifepo_temps_batch(data->lifepo_bank_temps_avg_data);
	print_led_current_batch(data->lifepo_current_avg_data);
	print_lifepo_current_batch(data->lifepo_current_avg_data);
	print_lifepo_volts_batch(data->lifepo_volts_avg_data);
	print_magnetometer_batch(data->mag_before_data);
	print("---------END LED COMPARISON Data---------\n");
}

void print_low_power_data(low_power_data_t* data, int i) {
	print_stack_type_header("Low Power Data Packet", i, data->timestamp, data->transmitted);
	print_satellite_state_history_batch(data->satellite_history);
	print_lion_volts_batch(data->lion_volts_data);
	print_lion_current_batch(data->lion_current_data);
	print_lion_temps_batch(data->lion_temps_data);
	print_panelref_lref_batch(data->panelref_lref_data);
	print_bat_charge_dig_sigs_batch(data->bat_charge_dig_sigs_data);
	print_ir_object_temps_batch(data->ir_obj_temps_data);
	print_gyro_batch(data->gyro_data);
}

void print_sat_error(sat_error_t* err, int i) {
	print("%2d: error (%s): loc=%s (%d)\t code=%s (%d)\t @ %d\n", i, 
		is_priority_error(*err) ? "priority" : "normal  ", 
		get_eloc_str(err), 
		err->eloc, 
		get_ecode_str(err), 
		err->ecode & 0b01111111,
		err->timestamp);
}


/************************************************************************/
/* String conversion funcs                                              */
/************************************************************************/
const char* get_sat_state_str(sat_state_t state) {
	switch (state) {
		case HELLO_WORLD:			return "HELLO_WORLD          ";
		case IDLE_FLASH:			return "IDLE_FLASH		     ";
		case IDLE_NO_FLASH:			return "IDLE_NO_FLASH		 ";
		case INITIAL:				return "INITIAL				 ";
		case ANTENNA_DEPLOY:		return "ANTENNA_DEPLOY		 ";
		case LOW_POWER:				return "LOW_POWER			 ";
		default: return "[invalid]";
	}
}

const char* get_task_str(task_type_t task) {
	switch (task) {
		case WATCHDOG_TASK:				return "WATCHDOG_TASK              ";
		case STATE_HANDLING_TASK:		return "STATE_HANDLING_TASK        ";
		case ANTENNA_DEPLOY_TASK:		return "ANTENNA_DEPLOY_TASK        ";
		case BATTERY_CHARGING_TASK:		return "BATTERY_CHARGING_TASK	   ";
		case TRANSMIT_TASK:				return "TRANSMIT_TASK              ";
		case FLASH_ACTIVATE_TASK:		return "FLASH_ACTIVATE_TASK        ";
		case IDLE_DATA_TASK:			return "IDLE_DATA_TASK             ";
		case LOW_POWER_DATA_TASK:		return "LOW_POWER_DATA_TASK        ";
		case ATTITUDE_DATA_TASK:		return "ATTITUDE_DATA_TASK         ";
		case PERSISTENT_DATA_BACKUP_TASK:return "PERSISTENT_DATA_BACKUP_TASK";
		default: return "[invalid]";
	}
}

const char* get_task_state_str(eTaskState state) {
	switch (state) {
		case eRunning:		return "eRunning   ";
		case eSuspended:	return "eSuspended ";
		case eDeleted:		return "eDeleted   ";
		case eBlocked:		return "eBlocked   ";
		case eReady:		return "eReady     ";
		default:			return "[invalid]";
	}
}

const char* get_msg_type_str(msg_data_type_t msg_type) {
	switch (msg_type) {
		case IDLE_DATA:			return "IDLE_DATA     ";
		case ATTITUDE_DATA:		return "ATTITUDE_DATA ";
		case FLASH_DATA:		return "FLASH_DATA    ";
		case FLASH_CMP_DATA:	return "FLASH_CMP_DATA";
		case LOW_POWER_DATA:	return "LOW_POWER_DATA";
		case NUM_MSG_TYPE:
		default:				return "[invalid]     ";
	}
}

static uint16_t get_task_stack_size(task_type_t task) {
	switch (task) {
		case WATCHDOG_TASK: return TASK_WATCHDOG_STACK_SIZE;
		case STATE_HANDLING_TASK: return TASK_STATE_HANDLING_STACK_SIZE;
		case ANTENNA_DEPLOY_TASK: return TASK_ANTENNA_DEPLOY_STACK_SIZE;
		case BATTERY_CHARGING_TASK: return TASK_BATTERY_CHARGING_STACK_SIZE;
		case TRANSMIT_TASK: return TASK_TRANSMIT_STACK_SIZE;
		case FLASH_ACTIVATE_TASK: return TASK_FLASH_ACTIVATE_STACK_SIZE;
		case IDLE_DATA_TASK: return TASK_IDLE_DATA_RD_STACK_SIZE;
		case LOW_POWER_DATA_TASK: return TASK_LOW_POWER_DATA_RD_STACK_SIZE;
		case ATTITUDE_DATA_TASK: return TASK_ATTITUDE_DATA_RD_STACK_SIZE;
		case PERSISTENT_DATA_BACKUP_TASK: return TASK_PERSISTENT_DATA_BACKUP_STACK_SIZE;
		default: return -1;
	}
}

static uint32_t get_task_freq(task_type_t task) {
	switch (task) {
		case WATCHDOG_TASK: return WATCHDOG_TASK_FREQ;
		case STATE_HANDLING_TASK: return STATE_HANDLING_TASK_FREQ;
		case ANTENNA_DEPLOY_TASK: return ANTENNA_DEPLOY_TASK_WATCHDOG_TIMEOUT;
		case BATTERY_CHARGING_TASK: return BATTERY_CHARGING_TASK_FREQ;
		case TRANSMIT_TASK: return low_power_active() ? TRANSMIT_TASK_LESS_FREQ : TRANSMIT_TASK_FREQ;
		case FLASH_ACTIVATE_TASK: return FLASH_ACTIVATE_TASK_FREQ;
		case IDLE_DATA_TASK: return IDLE_DATA_TASK_FREQ;
		case LOW_POWER_DATA_TASK: return LOW_POWER_DATA_TASK_FREQ;
		case ATTITUDE_DATA_TASK: return ATTITUDE_DATA_TASK_FREQ;
		case PERSISTENT_DATA_BACKUP_TASK: return PERSISTENT_DATA_BACKUP_TASK_FREQ;
		default: return -1;
	}
}

const char* get_eloc_str(sat_error_t* err) {
	// regex to generate: (ELOC_\w+)\s=\s+(\d+). -> case $2: return "$1";
	switch (err->eloc) {
	case 0: return "ELOC_NO_ERROR";

	case 1: return "ELOC_IR_POS_Y";
	case 2: return "ELOC_IR_NEG_X";
	case 3: return "ELOC_IR_NEG_Y";
	case 4: return "ELOC_IR_POS_X";
	case 5: return "ELOC_IR_NEG_Z";
	case 6: return "ELOC_IR_POS_Z";

	case 7: return "ELOC_PD_POS_Y";
	case 8: return "ELOC_PD_NEG_X";
	case 9: return "ELOC_PD_NEG_Y";
	case 10: return "ELOC_PD_POS_X";
	case 11: return "ELOC_PD_NEG_Z";
	case 12: return "ELOC_PD_POS_Z";

	case 13: return "ELOC_TEMP_LF_1";
	case 14: return "ELOC_TEMP_LF_2";
	case 15: return "ELOC_TEMP_L_1";
	case 16: return "ELOC_TEMP_L_2";
	case 17: return "ELOC_TEMP_LED_1";
	case 18: return "ELOC_TEMP_LED_2";
	case 19: return "ELOC_TEMP_LED_3";
	case 20: return "ELOC_TEMP_LED_4";

	case 21: return "ELOC_RADIO_TEMP";

	case 22: return "ELOC_IMU_ACC";
	case 23: return "ELOC_IMU_GYRO";
	case 24: return "ELOC_IMU_MAG";

	case 25: return "ELOC_LED1SNS";
	case 26: return "ELOC_LED2SNS";
	case 27: return "ELOC_LED3SNS";
	case 28: return "ELOC_LED4SNS";
	case 29: return "ELOC_LFB1OSNS";
	case 30: return "ELOC_LFB1SNS";
	case 31: return "ELOC_LFB2OSNS";
	case 32: return "ELOC_LFB2SNS";
	case 33: return "ELOC_LF1REF";
	case 34: return "ELOC_LF2REF";
	case 35: return "ELOC_LF3REF";
	case 36: return "ELOC_LF4REF";
	case 37: return "ELOC_L1_REF";
	case 38: return "ELOC_L2_REF";

	case 39: return "ELOC_RADIO";

	case 40: return "ELOC_AD7991_BBRD";
	case 41: return "ELOC_AD7991_BBRD_L2_SNS";
	case 42: return "ELOC_AD7991_BBRD_L1_SNS";
	case 43: return "ELOC_AD7991_BBRD_L_REF";
	case 44: return "ELOC_AD7991_BBRD_PANEL_REF";
	case 45: return "ELOC_AD7991_CBRD";
	case 46: return "ELOC_AD7991_CBRD_3V6_REF";
	case 47: return "ELOC_AD7991_CBRD_3V6_SNS";
	case 48: return "ELOC_AD7991_CBRD_5V_REF";
	case 49: return "ELOC_AD7991_CBRD_3V3_REF";

	case 50: return "ELOC_TCA";
	case 51: return "ELOC_CACHED_PERSISTENT_STATE";
	case 52: return "ELOC_MRAM1_READ";
	case 53: return "ELOC_MRAM2_READ";
	case 54: return "ELOC_MRAM_READ";
	case 55: return "ELOC_MRAM1_WRITE";
	case 56: return "ELOC_MRAM2_WRITE";
	case 57: return "ELOC_MRAM_WRITE";
	case 58: return "ELOC_5V_REF";
	case 59: return "ELOC_STATE_HANDLING";
	case 60: return "ELOC_BAT_CHARGING";
	case 61: return "ELOC_ANTENNA_DEPLOY";
	case 62: return "ELOC_WATCHDOG";
	case 63: return "ELOC_IMU_TEMP";
	case 64: return "ELOC_VERIFY_REGS";

	case 65: return "ELOC_IDLE_DATA";
	case 66: return "ELOC_ATTITUDE_DATA";
	case 67: return "ELOC_FLASH"; // both flash and flash_cmp (for now)
	case 68: return "ELOC_LOW_POWER_DATA";
	case 69: return "ELOC_EQUISTACK_GET";
	case 70: return "ELOC_EQUISTACK_PUT";

	case 71: return "ELOC_BOOTLOADER";
	case 72: return "ELOC_RTOS";

	case 73: return "ELOC_BAT_L1";
	case 74: return "ELOC_BAT_L2";
	case 75: return "ELOC_BAT_LFB1";
	case 76: return "ELOC_BAT_LFB2";
	case 77: return "ELOC_BAT_CHARGING_SWITCH_1";
	case 78: return "ELOC_BAT_CHARGING_SWITCH_2";
	case 79: return "ELOC_BAT_CHARGING_SWITCH_3";
	case 80: return "ELOC_BAT_CHARGING_SWITCH_4";
	case 81: return "ELOC_BAT_CHARGING_SWITCH_5";
	case 82: return "ELOC_BAT_CHARGING_SWITCH_6";
	case 83: return "ELOC_BAT_CHARGING_SWITCH_7";
	case 84: return "ELOC_BAT_CHARGING_SWITCH_8";
	case 85: return "ELOC_BAT_CHARGING_SWITCH_9";
	case 86: return "ELOC_IR_POW";
	case 87: return "ELOC_RADIO_KILLTIME";
	case 88: return "ELOC_RADIO_TRANSMIT";
	case 89: return "ELOC_RADIO_POWER";

	case 90: return "ELOC_IMU_INIT";
	case 91: return "ELOC_IMU_GYRO_INIT";
	case 92: return "ELOC_IMU_ACCEL_INIT";
	case 93: return "ELOC_IMU_MAG_INIT";
	default: return "[error loc not added to sys test]";
	}
}

const char* get_ecode_str(sat_error_t* err) {
	// regex to generate: (ECODE_\w+)\s=\s+(\d+). -> case $2: return "$1";
	switch (err->ecode & 0b01111111) {
	case 0: return "ECODE_OK";
	case 1: return "ECODE_VALID_DATA";
	case 2: return "ECODE_NO_CHANGE";
	case 3: return "ECODE_ABORTED";
	case 4: return "ECODE_BUSY";
	case 5: return "ECODE_SUSPEND";
	case 6: return "ECODE_IO";
	case 7: return "ECODE_REQ_FLUSHED";
	case 8: return "ECODE_TIMEOUT";
	case 9: return "ECODE_BAD_DATA";
	case 10: return "ECODE_NOT_FOUND";
	case 11: return "ECODE_UNSUPPORTED_DEV";
	case 12: return "ECODE_NO_MEMORY";
	case 13: return "ECODE_INVALID_ARG";
	case 14: return "ECODE_BAD_ADDRESS";
	case 15: return "ECODE_BAD_FORMAT";
	case 16: return "ECODE_BAD_FRQ";
	case 17: return "ECODE_DENIED";
	case 18: return "ECODE_ALREADY_INITIALIZED";
	case 19: return "ECODE_OVERFLOW";
	case 20: return "ECODE_NOT_INITIALIZED";
	case 21: return "ECODE_SAMPLERATE_UNAVAILABLE";
	case 22: return "ECODE_RESOLUTION_UNAVAILABLE";
	case 23: return "ECODE_BAUDRATE_UNAVAILABLE";
	case 24: return "ECODE_PACKET_COLLISION";
	case 25: return "ECODE_PROTOCOL";
	case 26: return "ECODE_PIN_MUX_INVALID";

	/**** CUSTOM ****/
	case 27: return "ECODE_READING_HIGH";
	case 28: return "ECODE_READING_LOW";
	case 29: return "ECODE_OUT_OF_BOUNDS";
	case 30: return "ECODE_SIGNAL_LOST";

	case 31: return "ECODE_CONFIRM_TIMEOUT";
	case 32: return "ECODE_INCONSISTENT_DATA";
	case 33: return "ECODE_UNEXPECTED_CASE";
	case 34: return "ECODE_WATCHDOG_EARLY_WARNING";
	case 35: return "ECODE_WATCHDOG_RESET";
	case 36: return "ECODE_WATCHDOG_DID_KICK";
	case 37: return "ECODE_EXCESSIVE_SUSPENSION";

	case 38: return "ECODE_CRIT_ACTION_MUTEX_TIMEOUT";
	case 39: return "ECODE_I2C_IRPOW_MUTEX_TIMEOUT";
	case 40: return "ECODE_PROC_ADC_MUTEX_TIMEOUT";
	case 41: return "ECODE_HW_STATE_MUTEX_TIMEOUT";
	case 42: return "ECODE_USART_MUTEX_TIMEOUT";
	case 43: return "ECODE_SPI_MUTEX_TIMEOUT";
	case 44: return "ECODE_BAT_CHARGING_MUTEX_TIMEOUT";
	case 45: return "ECODE_WATCHDOG_MUTEX_TIMEOUT";
	case 46: return "ECODE_EQUISTACK_MUTEX_TIMEOUT";
	case 47: return "ECODE_ALL_MUTEX_TIMEOUT";

	case 48: return "ECODE_REWROTE_PROG_MEM";
	case 49: return "ECODE_STACK_OVERFLOW";
	case 50: return "ECODE_DET_ALREADY_HIGH";

	case 51: return "ECODE_BAT_NOT_DISCHARGING";
	case 52: return "ECODE_BAT_NOT_NOT_DISCHARGING";
	case 53: return "ECODE_BAT_NOT_CHARGING";
	case 54: return "ECODE_BAT_NOT_NOT_CHARGING";
	case 55: return "ECODE_BAT_NOT_DISCHARGING_RESTART";
	case 56: return "ECODE_BAT_FAULT";
	case 57: return "ECODE_BAT_NOT_FULL_FOR_WHILE";
	case 58: return "ECODE_BAT_LOW_VOLTAGE_FOR_WHILE";
	case 59: return "ECODE_RECOMMISSION";
	case 60: return "ECODE_ALL_SAME_VAL";
	case 61: return "ECODE_CORRUPTED";
	case 62: return "ECODE_CORRUPTED_FATAL";
	case 63: return "ECODE_INVALID_STATE_CHANGE";
	case 64: return "ECODE_TIMESTAMP_WRAPAROUND";
	case 65: return "ECODE_INCONSISTENT_STATE";
	case 66: return "ECODE_PWM_CUR_LOW_ON_DEPLOY";
	case 67: return "ECODE_PWM_CUR_LOW_ON_MAX_CYCLE";
	case 68: return "ECODE_PWM_CUR_VERY_LOW_ON_DEPLOY";
	case 69: return "ECODE_PWM_CUR_VERY_LOW_ON_MAX_CYCLE";
	case 70: return "ECODE_SOFTWARE_RESET";
	case 71: return "ECODE_SAT_RESET";

	case 72: return "ECODE_BAT_LI_TIMEOUT";
	case 73: return "ECODE_BAT_LF_TIMEOUT";
	case 74: return "ECODE_IR_POW_IN_USE_ON_STATE_CHANGE";
	case 75: return "ECODE_UPLINK_REBOOT";
	case 76: return "ECODE_UPLINK_KILL3DAYS";
	case 77: return "ECODE_UPLINK_KILL1WEEK";
	case 78: return "ECODE_UPLINK_KILLFOREVER";
	case 79: return "ECODE_UPLINK_REVIVED";
	case 80: return "ECODE_P1_NOT_DEPLOYED";
	case 81: return "ECODE_P2_NOT_DEPLOYED";
	case 82: return "ECODE_P3_NOT_DEPLOYED";
	case 83: return "ECODE_IRPOW_SEM_TOO_MANY_USERS";
	case 84: return "ECODE_IRPOW_SEM_TOO_FEW_USERS";
	case 85: return "ECODE_I2C_BUS_ERROR";
	case 86: return "ECODE_BAT_LF_CELLS_UNBALANCED";
	case 87: return "ECODE_VALID_STATE_CHANGE";
	default: return "[error code not added to sys test]";
	}
}

/************************************************************************/
/* System test                                                          */
/************************************************************************/

void print_errors(int max_num) {
	print_equistack(&error_equistack, print_sat_error, "Error Stack", max_num);
}

// prints the given equistack using the given element-wise string building method
void print_equistack(equistack* stack, void (*elm_print)(void*, int), const char* header, int max_num) {
	print("\n==============%s==============\n", header);
	print("size: %d/%d \t top: %d \t bottom: %d\n" ,
		stack->cur_size, stack->max_size - 1, stack->top_index, stack->bottom_index); // -1 for staged area
	print("data (max n=%d):\n", max_num);
	if (max_num == -1) 
		max_num = stack->cur_size;
	for (int i = 0; i < min(stack->cur_size, max_num); i++) {
		(*elm_print)(equistack_Get(stack, i), i);
	}
}

static void print_equistacks(void) {
	print("\n==============Equistack Dump==============\n");
	int max_size = -1;
	if (only_print_recent_data) {
		max_size = 1;
	}
	// note: apparently C can't use void* as generic pointers if they're function pointer args... (sigh)
	print_equistack(&idle_readings_equistack,		print_idle_data,		"Idle Data Stack",			max_size);
	print_equistack(&attitude_readings_equistack,	print_attitude_data,	"Attitude Data Stack",		max_size);
	print_equistack(&flash_readings_equistack,		print_flash_data,		"Flash Data Stack",			max_size);
	print_equistack(&flash_cmp_readings_equistack,	print_flash_cmp_data,	"Flash Cmp Data Stack",		max_size);
	print_equistack(&low_power_readings_equistack,	print_low_power_data,	"Low Power Data Stack",		max_size);
}

void print_task_info(void) {
	print("\n\n===========Task Information===========\n");
	print("state consistency: %s\n", 
		check_task_state_consistency() ? "consistent": "!! INCONSISTENT TASK STATES !!");
	for (int task = 0; task < NUM_TASKS; task++) {
		eTaskState task_state = eTaskGetState(*(task_handles[task]));
		uint16_t stack_space_left = uxTaskGetStackHighWaterMark(*task_handles[task]) * sizeof(portSTACK_TYPE);
		uint16_t stack_size = get_task_stack_size(task);
		uint16_t stack_space_available = stack_size * sizeof(portSTACK_TYPE);
		// note watchdog task as checked out in sys test so it doesn't look like something's up
		bool checked_in = (task == WATCHDOG_TASK) ? false : _get_task_checked_in(task);
		uint32_t last_check_in = _get_task_checked_in_time(task);
		uint32_t task_freq = get_task_freq(task);
		
		print("%s: %s (%s) LRT: %5d (%3d%%) stack: %4d / %4d (%3d%%)\n", 
			get_task_str(task), 
			get_task_state_str(task_state),
			checked_in ? "checked in " : "checked out",
			checked_in ? (xTaskGetTickCount() - last_check_in) : 0,
			checked_in ? (100 * (xTaskGetTickCount() - last_check_in) / task_freq) : 0,
			stack_space_available - stack_space_left,
			stack_space_available,
			(100 * (stack_space_available - stack_space_left)) / stack_space_available);
	}
}

void print_cur_data_buf(uint8_t* cur_data_buf) {
	print("\n\n============Current Data============\n");
	static uint8_t cmp_buf[MSG_CUR_DATA_LEN];
	memset(cmp_buf, 0, sizeof(cmp_buf));
	if (memcmp(cur_data_buf, cmp_buf, MSG_CUR_DATA_LEN) != 0) {
		print("secs to next flash: %d\n", cur_data_buf[0]);
		print("reboot count: %d\n",		cur_data_buf[1]);
		print_lion_volts_batch(			&(cur_data_buf[2]));
		print_lion_current_batch(		&(cur_data_buf[4]));
		print_lion_temps_batch(			&(cur_data_buf[6]));
		print_panelref_lref_batch(		&(cur_data_buf[8]));
		print_bat_charge_dig_sigs_batch(cur_data_buf[11]<<8 | cur_data_buf[10]);
		print_lifepo_volts_batch(		&(cur_data_buf[12]));
	} else {
		print("(none available (or all zeros)");
	}
}

void rtos_system_test(void) {
	print("\n\n==============RTOS System Test==============\n");
	#ifndef RTOS_SYSTEM_TEST_SUMMARY
	print_cur_data_buf(_get_cur_data_buf());
	print_equistacks();
	int max_num_errors = -1;
	#else
	int max_num_errors = 10;
	#endif
	print("\n\n==============System Info==============\n");
	print("timestamp: \t%ds \t(%dmin)\n", get_current_timestamp(), get_current_timestamp()/60);
	print("ticks:	  \t%d\n", xTaskGetTickCount());
	print("sat state: \t%s\n", get_sat_state_str(get_sat_state()));
	print("reboot #:  \t%d\n", cache_get_reboot_count());
	print("num errors:\t%d\n", error_equistack.cur_size);
	print("most recent errs: ");
	print_errors(max_num_errors);
	print_task_info();
	print("====================End=====================\n\n");
}