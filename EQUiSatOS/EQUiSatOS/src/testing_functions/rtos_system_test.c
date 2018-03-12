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
static uint16_t untruncate(uint8_t val, sig_id_t sig) {
	uint16_t u16 = ((uint8_t)val) << 8;
	return u16 / get_line_m_from_signal(sig) - get_line_b_from_signal(sig);
}

void print_accel_batch(accelerometer_batch batch) {
	print("accel\n");
	print("\tx: %d\t%d mV\n", batch[0], untruncate(batch[0], S_ACCEL));
	print("\ty: %d\t%d mV\n", batch[1], untruncate(batch[1], S_ACCEL));
	print("\tz: %d\t%d mV\n", batch[2], untruncate(batch[2], S_ACCEL));
}
void print_gyro_batch(gyro_batch batch) {
	print("gyro\n");
	print("\tx: %d\t%d mV\n", batch[0], untruncate(batch[0], S_GYRO));
	print("\ty: %d\t%d mV\n", batch[1], untruncate(batch[1], S_GYRO));
	print("\tz: %d\t%d mV\n", batch[2], untruncate(batch[2], S_GYRO));
}
void print_magnetometer_batch(magnetometer_batch batch) {
	print("mag\n");
	print("\tx: %d\t%d mV\n", batch[0], untruncate(batch[0], S_MAG));
	print("\ty: %d\t%d mV\n", batch[1], untruncate(batch[1], S_MAG));
	print("\tz: %d\t%d mV\n", batch[2], untruncate(batch[2], S_MAG));
}
void print_ir_ambient_temps_batch(ir_ambient_temps_batch batch) {
	print("ir ambs\n");
	for (int i = 0; i < 6; i++) {
		print("\t%d: %d\t%d C\n", i+1, batch[i], dataToTemp(untruncate(batch[i], S_IR_AMB)));
	}
}
void print_ir_object_temps_batch(ir_object_temps_batch batch) {
	print("ir objs\n");
	for (int i = 0; i < 6; i++) {
		print("\t%d: %d\t%d C\n", i+1, batch[i], dataToTemp(batch[i]));
	}
}
void print_lion_volts_batch(lion_volts_batch batch) {
	print("lion volts 1: %d %d mV\n", batch[0], untruncate(batch[0], S_L_VOLT));
	print("lion volts 2: %d %d mV\n", batch[1], untruncate(batch[1], S_L_VOLT));
}
void print_lion_current_batch(lion_current_batch batch) {
	print("lion SNS 1: %d %d mV\n", batch[0], untruncate(batch[0], S_L_SNS));
	print("lion SNS 2: %d %d mV\n", batch[1], untruncate(batch[1], S_L_SNS));
}
void print_lion_temps_batch(lion_temps_batch batch) {
	print("lion temps 1: %d %d C\n", batch[0], dataToTemp(untruncate(batch[0], S_L_TEMP)));
	print("lion temps 2: %d %d C\n", batch[1], dataToTemp(untruncate(batch[1], S_L_TEMP)));
}
void print_led_temps_batch(led_temps_batch batch) {
	print("led temps\n");
	for (int i = 0; i < 4; i++) {
		print("\t%d: %d\t%d\n", i+1, batch[i], dataToTemp(untruncate(batch[i], S_LED_TEMP)));
	}
}
void print_lifepo_temps_batch(lifepo_bank_temps_batch batch) {
	print("lifepo bank temps\n");
	print("\t1: %d\t%d C\n", batch[0], dataToTemp(untruncate(batch[0], S_LF_TEMP)));
	print("\t2: %d\t%d C\n", batch[1], dataToTemp(untruncate(batch[1], S_LF_TEMP)));
}
void print_lifepo_volts_batch(lifepo_volts_batch batch) {
	print("lifepo volts\n");
	for (int i = 0; i < 4; i++) {
		print("\t%d: %d\t%d mv\n", i+1, batch[i], untruncate(batch[i], S_LF_VOLT));
	}
}
void print_lifepo_current_batch(lifepo_current_batch batch) {
	print("lifepo SNS\n");
	print("\tbank1: %d\t%d mV\n", batch[0], untruncate(batch[0], S_LF_SNS_REG));
	print("\tbank2: %d\t%d mV\n\n", batch[2], untruncate(batch[2], S_LF_SNS_REG));
	print("lifepo OSNS\n");
	print("\tbank1: %d\t%d mV\n", batch[1], untruncate(batch[1], S_LF_OSNS_REG));
	print("\tbank2: %d\t%d mV\n\n", batch[3], untruncate(batch[3], S_LF_OSNS_REG));
}
void print_led_current_batch(led_current_batch batch) {
	print("led current\n");
	for (int i = 0; i < 4; i++) {
		print("\t%d: %d\t%d mV\n", i+1, batch[i], untruncate(batch[i], S_LED_SNS));
	}
}
void print_panelref_lref_batch(panelref_lref_batch batch) {
	print("refs: PANELREF: %d %d mV\nL_REF: %d %d mV\n", batch[0], untruncate(batch[0], S_PANELREF), batch[1], untruncate(batch[1], S_LREF));
}
void print_radio_temp_batch(radio_temp_batch batch) {
	print("radio temp: %d\t%d C\n", batch, dataToTemp(untruncate(batch, S_RAD_TEMP)));
}
void print_imu_temp_batch(imu_temp_batch batch) {
	print("imu temp: %d\t%d C\n", batch, dataToTemp(untruncate(batch, S_IMU_TEMP)));
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
void print_stack_type_header(const char* header, int i, uint32_t timestamp, bool transmitted) {
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
	print_stack_type_header("Flash Data Packet", i_global, data->timestamp, data->transmitted);
	print("---LED Temp Burst Data---\n");
	for (int i = 0; i < FLASH_DATA_ARR_LEN; i++) {
		print_led_temps_batch(data->led_temps_data[i]);
	}
	print("---LiFePo Temp Burst Data---\n");
	for (int i = 0; i < FLASH_DATA_ARR_LEN; i++) {
		print_lifepo_temps_batch(data->lifepo_bank_temps_data[i]);
	}
	print("---LiFePo Current Burst Data---\n");
	for (int i = 0; i < FLASH_DATA_ARR_LEN; i++) {
		print_lifepo_current_batch(data->lifepo_current_data[i]);
	}
	print("---LiFePo Volts Burst Data---\n");
	for (int i = 0; i < FLASH_DATA_ARR_LEN; i++) {
		print_lifepo_volts_batch(data->lifepo_volts_data[i]);
	}
	print("---Led Current Burst Data---\n");
	for (int i = 0; i < FLASH_DATA_ARR_LEN; i++) {
		print_led_current_batch(data->led_current_data[i]);
	}
	print("---Gyro Burst Data---\n");
	for (int i = 0; i < FLASH_DATA_ARR_LEN; i++) {
		print_gyro_batch(data->gyro_data[i]);
	}
}

void print_flash_cmp_data(flash_cmp_data_t* data, int i) {
	print_stack_type_header("Flash Compare Data Packet", i, data->timestamp, data->transmitted);
	print_led_temps_batch(data->led_temps_avg_data);
	print_lifepo_temps_batch(data->lifepo_bank_temps_avg_data);
	print_led_current_batch(data->lifepo_current_avg_data);
	print_lifepo_current_batch(data->lifepo_current_avg_data);
	print_lifepo_volts_batch(data->lifepo_volts_avg_data);
	print_magnetometer_batch(data->mag_before_data);
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
	print("%2d: error (%s): loc=%3d code=%3d @ %d\n", i, 
		is_priority_error(*err) ? "priority" : "normal  ", err->eloc, err->ecode & 0b01111111, err->timestamp);
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

uint16_t get_task_stack_size(task_type_t task) {
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

uint32_t get_task_freq(task_type_t task) {
	switch (task) {
		case WATCHDOG_TASK: return WATCHDOG_TASK_FREQ;
		case STATE_HANDLING_TASK: return STATE_HANDLING_TASK_FREQ;
		case ANTENNA_DEPLOY_TASK: return ANTENNA_DEPLOY_TASK_FREQ;
		case BATTERY_CHARGING_TASK: return BATTERY_CHARGING_TASK_FREQ;
		case TRANSMIT_TASK: return TRANSMIT_TASK_FREQ;
		case FLASH_ACTIVATE_TASK: return FLASH_ACTIVATE_TASK_FREQ;
		case IDLE_DATA_TASK: return IDLE_DATA_TASK_FREQ;
		case LOW_POWER_DATA_TASK: return LOW_POWER_DATA_TASK_FREQ;
		case ATTITUDE_DATA_TASK: return ATTITUDE_DATA_TASK_FREQ;
		case PERSISTENT_DATA_BACKUP_TASK: return PERSISTENT_DATA_BACKUP_TASK_FREQ;
		default: return -1;
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
		stack->cur_size, stack->max_size, stack->top_index, stack->bottom_index);
	print("data (max n=%d):\n", max_num);
	if (max_num == -1) 
		max_num = stack->cur_size;
	for (int i = 0; i < min(stack->cur_size, max_num); i++) {
		(*elm_print)(equistack_Get(stack, i), i);
	}
}

void print_equistacks(void) {
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
		bool checked_in = _get_task_checked_in(task);
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
	print("secs to next flash: %d\n", cur_data_buf[0]);
	print("reboot count: %d\n",		cur_data_buf[1]);
	print_lion_volts_batch(			&(cur_data_buf[2]));
	print_lion_current_batch(		&(cur_data_buf[4]));
	print_lion_temps_batch(			&(cur_data_buf[6]));
	print_panelref_lref_batch(		&(cur_data_buf[8]));
	print_bat_charge_dig_sigs_batch(cur_data_buf[10]);
	print_lifepo_volts_batch(		&(cur_data_buf[12]));
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