/*
 * sensor_read_commands.c
 *
 * Created: 11/1/2016 8:18:16 PM
 *  Author: mckenna
 */

#include "sensor_read_commands.h"

/* NOTE: the "batch" value passed into these functions are generally arrays, so are passed by reference */

static uint8_t IRs[6] = {
	MLX90614_FLASHPANEL_V6_2_1,
	MLX90614_TOPPANEL_V4_2,
	MLX90614_ACCESSPANEL_V3_1,
	MLX90614_SIDEPANEL_V4_2,
	MLX90614_SIDEPANEL_V4_3,
	MLX90614_SIDEPANEL_V4_4
};

static uint8_t IR_ELOCS[6] = {
	ELOC_IR_1,
	ELOC_IR_2,
	ELOC_IR_3,
	ELOC_IR_4,
	ELOC_IR_5,
	ELOC_IR_6
};

static uint8_t TEMP_ELOCS[8] = {
	ELOC_TEMP_1,
	ELOC_TEMP_2,
	ELOC_TEMP_3,
	ELOC_TEMP_4,
	ELOC_TEMP_5,
	ELOC_TEMP_6,
	ELOC_TEMP_7,
	ELOC_TEMP_8
};

static uint8_t PD_ELOCS[6] = {
	ELOC_PD_1,
	ELOC_PD_2,
	ELOC_PD_3,
	ELOC_PD_4,
	ELOC_PD_5,
	ELOC_PD_6,
};

static struct adc_module adc_instance;
static enum status_code sc;

static void commands_read_adc(uint16_t* dest, uint8_t pin, uint8_t eloc, bool priority) {
	sc = configure_adc(&adc_instance, pin);
	log_if_error(eloc, sc, priority);
	sc = read_adc(adc_instance, dest);
	log_if_error(eloc, sc, priority);
}

void read_ir_object_temps_batch(ir_object_temps_batch batch) {
	for (int i = 0; i < 6; i ++) {
		uint16_t obj1;
		sc = MLX90614_read2ByteValue(IRs[i] / 2, OBJ1, &obj1);
		log_if_error(IR_ELOCS[i], sc, false);
		uint16_t obj2;
		sc = MLX90614_read2ByteValue(IRs[i] / 2, OBJ2, &obj2);
		log_if_error(IR_ELOCS[i], sc, false);
		batch[i] = (obj1 + obj2) / 2;
	}
}

void read_ir_ambient_temps_batch(ir_ambient_temps_batch batch) {
	for (int i = 0; i < 6; i++) {
		uint16_t amb;
		sc = MLX90614_read2ByteValue(IRs[i] / 2, AMBIENT, &amb);
		log_if_error(IR_ELOCS[i], sc, false);
		batch[i] = amb;
	}
}

void read_lion_volts_batch(lion_volts_batch batch) {
	commands_read_adc(&batch[0], P_AI_L1_REF, ELOC_L1_REF, false);
	commands_read_adc(&batch[1], P_AI_L2_REF, ELOC_L2_REF, false);
}

void read_lion_current_batch(lion_current_batch batch) {
	uint16_t results[4];
	// 0 is the battery board
	sc = AD7991_read_all(results, AD7991_ADDR_0);
	log_if_error(ELOC_AD7991_0, sc, true);
	// battery 1 voltage is Vin1
	batch[0] = results[1];
	// battery 2 voltage is Vin0
	batch[1] = results[0];
}

void read_led_temps_batch(led_temps_batch batch) {
	// LTC addresses may need to be disabled
	for (int i = 0; i < 4; i++) {
		uint8_t rs;
		sc = LTC1380_channel_select(TEMP_MULTIPLEXER_I2C, i, &rs);
		log_if_error(TEMP_ELOCS[i], sc, true);
		batch[i] = rs;
	}
}

void read_lifepo_current_batch(lifepo_current_batch batch) {
	commands_read_adc(&batch[0], P_AI_LFB1SNS, ELOC_LFB1SNS, true);
	commands_read_adc(&batch[1], P_AI_LFB1OSNS, ELOC_LFB1OSNS, true);
	commands_read_adc(&batch[2], P_AI_LFB2SNS, ELOC_LFB2SNS, true);
	commands_read_adc(&batch[3], P_AI_LFB2OSNS, ELOC_LFB2OSNS, true);
}

void read_lifepo_volts_batch(lifepo_volts_batch batch) {
	commands_read_adc(&batch[0], P_AI_LF1REF, ELOC_LF1REF, true);
	commands_read_adc(&batch[1], P_AI_LF2REF, ELOC_LF2REF, true);
	commands_read_adc(&batch[2], P_AI_LF3REF, ELOC_LF3REF, true);
	commands_read_adc(&batch[3], P_AI_LF4REF, ELOC_LF4REF, true);
}

void read_lifepo_temps_batch(lifepo_bank_temps_batch batch) {





	// TODO





}

void read_pdiode_batch(pdiode_batch batch) {
	for (int i = 0; i < 6; i++) {
		uint8_t rs;
		sc = LTC1380_channel_select(PHOTO_MULTIPLEXER_I2C, i, &rs);
		log_if_error(PD_ELOCS[i], sc, false);
		batch[i] = rs;
	}
}

void read_lion_temps_batch(lion_temps_batch batch) {
	for (int i = 4; i < 8; i++) {
		uint8_t rs;
		sc = LTC1380_channel_select(TEMP_MULTIPLEXER_I2C, i, &rs);
		log_if_error(TEMP_ELOCS[i], sc, false);
		batch[i - 4] = rs;
	}
}

void read_accel_batch(accelerometer_batch accel_batch) {
	sc = MPU9250_read_acc((uint16_t*) accel_batch);
	log_if_error(ELOC_IMU_ACC, sc, false);
}

void read_gyro_batch(gyro_batch gyr_batch) {
	sc = MPU9250_read_gyro((uint16_t*) gyr_batch);
	log_if_error(ELOC_IMU_GYRO, sc, false);
}

void read_magnetometer_batch(magnetometer_batch batch) {
	sc = MPU9250_read_mag((uint16_t*) batch);
	log_if_error(ELOC_IMU_MAG, sc, false);
}

void read_led_current_batch(led_current_batch batch) {
	commands_read_adc(&batch[0], P_AI_LED1SNS, ELOC_LED1SNS, true);
	commands_read_adc(&batch[1], P_AI_LED2SNS, ELOC_LED2SNS, true);
	commands_read_adc(&batch[2], P_AI_LED3SNS, ELOC_LED3SNS, true);
	commands_read_adc(&batch[3], P_AI_LED4SNS, ELOC_LED4SNS, true);
}

void read_radio_temp_batch(radio_temp_batch* batch) {
	// TODO: Tyler will implement later
	// eventually: XDL_get_temperature(batch);
}

void read_radio_volts_batch(radio_volts_batch batch) {
	// 3v6_ref and 3v6_sns
	uint16_t results[4];
	// 1 is the control board
	sc = AD7991_read_all(results, AD7991_ADDR_1);
	log_if_error(ELOC_AD7991_1, sc, false);
	// 3V6 voltage is Vin0
	batch[0] = results[0];
	// 3V6 current is Vin1
	batch[1] = results[1];
}

void read_bat_charge_volts_batch(bat_charge_volts_batch batch) {
	// analog voltages on spreadsheet
	uint16_t results[4];
	// 1 is the control board
	sc = AD7991_read_all(results, AD7991_ADDR_1);
	log_if_error(ELOC_AD7991_1, sc, false);
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

void read_rail_5v_batch(rail_5v_batch* batch) {

}
