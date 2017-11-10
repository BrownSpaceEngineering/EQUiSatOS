/*
 * sensor_read_commands.c
 *
 * Created: 11/1/2016 8:18:16 PM
 *  Author: mckenna
 */ 

#include "sensor_read_commands.h"

/* NOTE: the "batch" value passed into these functions are generally arrays, so are passed by reference */

uint8_t IRs[6] = {
	MLX90614_FLASHPANEL_V6_2_1,
	MLX90614_TOPPANEL_V4_2,
	MLX90614_ACCESSPANEL_V3_1,
	MLX90614_SIDEPANEL_V4_2,
	MLX90614_SIDEPANEL_V4_3,
	MLX90614_SIDEPANEL_V4_4
};
	
uint8_t IR_ELOCS[6] = {
	ELOC_IR_1,
	ELOC_IR_2,
	ELOC_IR_3,
	ELOC_IR_4,
	ELOC_IR_5,
	ELOC_IR_6
};

void read_ir_batch(ir_batch batch) {
	for (int i = 0; i < 6; i ++) {
		uint16_t obj1;
		enum status_code sc = MLX90614_read2ByteValue(IRs[i] / 2, OBJ1, &obj1);
		if (sc != STATUS_OK) {
			log_error(IR_ELOCS[i], atmel_to_equi_error(sc), false);
		}
		uint16_t obj2;
		sc = MLX90614_read2ByteValue(IRs[i] / 2, OBJ2, &obj2);
		if (sc != STATUS_OK) {
			log_error(IR_ELOCS[i], atmel_to_equi_error(sc), false);
		}
		batch[i] = (obj1 + obj2) / 2;
	}
}

void read_ir_temps_batch(ir_temps_batch batch) {
	for (int i = 0; i < 6; i++) {
		uint16_t amb;
		enum status_code sc = MLX90614_read2ByteValue(IRs[i] / 2, AMBIENT, &amb);
		if (sc != STATUS_OK) {
			log_error(IR_ELOCS[i], atmel_to_equi_error(sc), false);
		}
		batch[i] = amb;
	}
}

void read_lion_volts_batch(lion_volts_batch batch) {
	// readFromADC?
	struct adc_module adc_instance1;
	struct adc_module adc_instance2;
	uint16_t buf;
	enum status_code sc = configure_adc(&adc_instance1, P_AI_L1_REF);
	read_adc(adc_instance1, &buf);
	batch[0] = buf;
	sc = configure_adc(&adc_instance2, P_AI_L2_REF);
	read_adc(adc_instance2, &buf);
	batch[1] = buf;
}

void read_lion_current_batch(lion_current_batch batch) {
	uint16_t results[4];
	// 0 is the battery board
	enum status_code sc = AD7991_read_all(results, AD7991_ADDR_0);
	// battery 1 voltage is Vin1
	batch[0] = results[1];
	// battery 2 voltage is Vin0
	batch[1] = results[0];
}

void read_led_temps_batch(led_temps_batch batch) {
	// LTC addresses may need to be disabled
	for (int i = 0; i < 4; i++) {
		uint8_t rs;
		enum status_code sc = LTC1380_channel_select(TEMP_MULTIPLEXER_I2C, i, &rs);
		batch[i] = rs;
	}
}

void read_lifepo_current_batch(lifepo_current_batch batch) {
	struct adc_module adc_instance1;
	struct adc_module adc_instance2;
	struct adc_module adc_instance3;
	struct adc_module adc_instance4;
	uint16_t buf;
	// order is bank 1, bank 1 out, bank 2, bank 2 out
	configure_adc(&adc_instance1, P_AI_LFB1SNS);
	read_adc(adc_instance1, &buf);
	batch[0] = buf;
	configure_adc(&adc_instance2, P_AI_LFB1OSNS);
	read_adc(adc_instance2, &buf);
	batch[1] = buf;
	configure_adc(&adc_instance3, P_AI_LFB2SNS);
	read_adc(adc_instance3, &buf);
	batch[2] = buf;
	configure_adc(&adc_instance4, P_AI_LFB2OSNS);
	read_adc(adc_instance4, &buf);
	batch[3] = buf;
}

void read_lifepo_volts_batch(lifepo_volts_batch batch) {
	struct adc_module adc_instance1;
	struct adc_module adc_instance2;
	struct adc_module adc_instance3;
	struct adc_module adc_instance4;
	uint16_t buf;
	// order is bank 1, bank 1 out, bank 2, bank 2 out
	configure_adc(&adc_instance1, P_AI_LF1REF);
	read_adc(adc_instance1, &buf);
	batch[0] = buf;
	configure_adc(&adc_instance2, P_AI_LF2REF);
	read_adc(adc_instance2, &buf);
	batch[1] = buf;
	configure_adc(&adc_instance3, P_AI_LF3REF);
	read_adc(adc_instance3, &buf);
	batch[2] = buf;
	configure_adc(&adc_instance4, P_AI_LF4REF);
	read_adc(adc_instance4, &buf);
	batch[3] = buf;
}

void read_pdiode_batch(pdiode_batch batch) {
	for (int i = 0; i < 6; i++) {
		uint8_t rs;
		LTC1380_channel_select(PHOTO_MULTIPLEXER_I2C, i, &rs);
		batch[i] = rs;
	}
}

void read_bat_temp_batch(bat_temp_batch batch) {
	for (int i = 4; i < 8; i++) {
		uint8_t rs;
		LTC1380_channel_select(TEMP_MULTIPLEXER_I2C, i, &rs);
		batch[i - 4] = rs;
	}
}

void read_accel_batch(accelerometer_batch accel_batch) {
	enum status_code sc = MPU9250_read_acc((uint16_t*) accel_batch);
	if (sc != STATUS_OK) {
		log_error(ELOC_IMU_ACC, atmel_to_equi_error(sc), false);
	}
}

void read_gyro_batch(gyro_batch gyr_batch) {
	enum status_code sc = MPU9250_read_gyro((uint16_t*) gyr_batch);
	if (sc != STATUS_OK) {
		log_error(ELOC_IMU_GYRO, atmel_to_equi_error(sc), false);
	}
}

void read_magnetometer_batch(magnetometer_batch batch) {
	enum status_code sc = MPU9250_read_mag((uint16_t*) batch);
	if (sc != STATUS_OK) {
		log_error(ELOC_IMU_MAG, atmel_to_equi_error(sc), false);
	}
}

void read_led_current_batch(led_current_batch batch) {
	struct adc_module adc_instance1;
	struct adc_module adc_instance2;
	struct adc_module adc_instance3;
	struct adc_module adc_instance4;
	uint16_t buf;
	// order is bank 1, bank 1 out, bank 2, bank 2 out
	configure_adc(&adc_instance1, P_AI_LED1SNS);
	read_adc(adc_instance1, &buf);
	batch[0] = buf;
	configure_adc(&adc_instance2, P_AI_LED2SNS);
	read_adc(adc_instance2, &buf);
	batch[1] = buf;
	configure_adc(&adc_instance3, P_AI_LED3SNS);
	read_adc(adc_instance3, &buf);
	batch[2] = buf;
	configure_adc(&adc_instance4, P_AI_LED4SNS);
	read_adc(adc_instance4, &buf);
	batch[3] = buf;
}

void read_radio_temp_batch(radio_temp_batch* batch) {
	// TODO: Tyler will implement later
	// eventually: XDL_get_temperature(batch);
}
	
void read_radio_volts_batch(radio_volts_batch batch) {
	// 3v6_ref and 3v6_sns
	uint16_t results[4];
	// 1 is the control board
	AD7991_read_all(results, AD7991_ADDR_1);
	// 3V6 voltage is Vin0
	batch[0] = results[0];
	// 3V6 current is Vin1
	batch[1] = results[1];
}

void read_bat_charge_volts_batch(bat_charge_volts_batch batch) {
	// analog voltages on spreadsheet
	uint16_t results[4];
	// 1 is the control board
	AD7991_read_all(results, AD7991_ADDR_1);
	// 5V voltage is Vin2
	batch[0] = results[2];
	// 3V3 current is Vin3
	batch[1] = results[3];
}

void read_bat_charge_dig_sigs_batch(bat_charge_dig_sigs_batch* batch) {
	// L1_RUN_CHG to LF_B2_OUTEN
	uint16_t accum = 0;
	setup_pin(false, P_L1_FAULTN);
	accum = accum & get_input(P_L1_FAULTN);
	setup_pin(false, P_L1_CHGN);
	accum = (accum << 1) & get_input(P_L1_CHGN);
	setup_pin(false, P_L1_ST);
	accum = (accum << 1) & get_input(P_L1_ST);
	setup_pin(false, P_L2_FAULTN);
	accum = (accum << 1) & get_input(P_L2_FAULTN);
	setup_pin(false, P_L2_CHGN);
	accum = (accum << 1) & get_input(P_L2_CHGN);
	setup_pin(false, P_L2_ST);
	accum = (accum << 1) & get_input(P_L2_ST);
	setup_pin(false, P_LF_B1_CHGN);
	accum = (accum << 1) & get_input(P_LF_B1_CHGN);
	setup_pin(false, P_LF_B1_FAULTN);
	accum = (accum << 1) & get_input(P_LF_B1_FAULTN);
	setup_pin(false, P_LF_B2_CHGN);
	accum = (accum << 1) & get_input(P_LF_B2_CHGN);
	setup_pin(false, P_LF_B2_FAULTN);
	accum = (accum << 1) & get_input(P_LF_B2_FAULTN);
	setup_pin(false, P_SPF_ST);
	accum = (accum << 1) & get_input(P_SPF_ST);
	
	*batch = accum;
}
	
void read_digital_out_batch(digital_out_batch* batch) {
	// yet to be defined, mapped to certain events
}

void read_imu_temp_batch(imu_temp_batch* batch) {
	
}

void read_rail_3v_batch(rail_3v_batch* batch) {
	
}

void read_rail_5v_batch(rail_5v_batch* batch) {
		
}