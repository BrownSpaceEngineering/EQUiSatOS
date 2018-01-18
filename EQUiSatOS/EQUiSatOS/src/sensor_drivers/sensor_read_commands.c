/*
 * sensor_read_commands.c
 *
 * Created: 11/1/2016 8:18:16 PM
 *  Author: jleiken
 */

#include "sensor_read_commands.h"

static uint8_t IR_ADDS[6] = {
	IR_FLASH,
	IR_SIDE1,
	IR_SIDE2,
	IR_RBF,
	IR_ACCESS,
	IR_TOP1
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

/* NOTE: the "batch" value passed into these functions are generally arrays, so are passed by reference */

static uint8_t truncate_16t(uint16_t src) {
	return src >> 8;
}

static void log_if_out_of_bounds(uint reading, uint low, uint high, uint8_t eloc, bool priority) {
	if (reading < low) {
		log_error(eloc, ECODE_READING_LOW, priority);
	} else if (reading > high) {
		log_error(eloc, ECODE_READING_HIGH, priority);
	}
}

static void commands_read_adc(uint16_t* dest, int pin, uint8_t eloc, uint low_bound, uint high_bound, bool priority) {
	sc = configure_adc(&adc_instance, pin);
	log_if_error(eloc, sc, priority);
	sc = read_adc_mV(adc_instance, dest);
	log_if_error(eloc, sc, priority);
	log_if_out_of_bounds(*dest, low_bound, high_bound, eloc, priority);
}

static void commands_read_adc_truncate(uint8_t* dest, int pin, uint8_t eloc, uint low_bound, uint high_bound, bool priority) {
	uint16_t read;
	sc = configure_adc(&adc_instance, pin);
	log_if_error(eloc, sc, priority);
	sc = read_adc_mV(adc_instance, &read);
	log_if_error(eloc, sc, priority);
	log_if_out_of_bounds(read, low_bound, high_bound, eloc, priority);
	*dest = truncate_16t(read);
}

void read_ir_object_temps_batch(ir_object_temps_batch batch) {
	for (int i = 0; i < 6; i ++) {
		uint16_t obj;
		sc = MLX90614_read_all_obj(IR_ADDS[i], &obj);
		log_if_error(IR_ELOCS[i], sc, false);
		log_if_out_of_bounds(obj, IR_OBJ_LOW, IR_OBJ_HIGH, IR_ELOCS[i], false);
		batch[i] = obj;
	}
}

void read_ir_ambient_temps_batch(ir_ambient_temps_batch batch) {
	for (int i = 0; i < 6; i++) {
		uint16_t amb;
		sc = MLX90614_read2ByteValue(IR_ADDS[i], AMBIENT, &amb);
		log_if_error(IR_ELOCS[i], sc, false);
		log_if_out_of_bounds(amb, IR_AMB_LOW, IR_AMB_HIGH, IR_ELOCS[i], false);
		batch[i] = truncate_16t(amb);
	}
}

// TODO: make sure this all looks okay
void read_lion_volts_batch(lion_volts_batch batch) {
	read_li_volts_precise(&(batch[0]), &(batch[1]));
	batch[0] = truncate_16t(batch[0]);
	batch[1] = truncate_16t(batch[1]);
}

void read_li_volts_precise(uint16_t* val_1, uint16_t* val_2) {
	uint16_t buf;
	commands_read_adc(&buf, P_AI_L1_REF, ELOC_L1_REF, L_VOLT_LOW, L_VOLT_HIGH, true);
	buf *= 2500;
	*val_1 = buf;

	commands_read_adc(&buf, P_AI_L2_REF, ELOC_L2_REF, L_VOLT_LOW, L_VOLT_HIGH, true);
	buf *= 2500;
	*val_2 = buf;
}

void read_lion_current_batch(lion_current_batch batch) {
	uint16_t results[4];
	sc = AD7991_read_all(results, AD7991_BATBRD);
	log_if_error(ELOC_AD7991_0, sc, true);
	// L1_SNS is Vin1
	batch[0] = truncate_16t(results[1]);
	log_if_out_of_bounds(results[1], L_CUR_LOW, L_CUR_HIGH, ELOC_AD7991_0_1, true);
	// L2_SNS is Vin0
	batch[1] = truncate_16t(results[0]);
	log_if_out_of_bounds(results[0], L_CUR_LOW, L_CUR_HIGH, ELOC_AD7991_0_0, true);
}

static void verify_5v_ref(void) {
	delay_ms(100); // ?
	uint8_t res_5v[2];
	// gets 5VREF in position 0
	read_bat_charge_volts_batch(res_5v);
	log_if_out_of_bounds(res_5v[0], FV_EN_LOW, FV_EN_HIGH, ELOC_5V_REF, true);
}

void read_led_temps_batch(led_temps_batch batch) {
	set_output(true, P_5V_EN);
	verify_5v_ref();
	for (int i = 4; i < 8; i++) {
		uint8_t rs8;
		sc = LTC1380_channel_select(TEMP_MULTIPLEXER_I2C, i, &rs8);
		log_if_error(TEMP_ELOCS[i], sc, true);
		commands_read_adc_truncate(&rs8, P_AI_TEMP_OUT, TEMP_ELOCS[i], LED_TEMP_LOW, LED_TEMP_HIGH, true);
		batch[i - 4] = rs8;
	}
	set_output(false, P_5V_EN);
}

void read_lifepo_current_batch(lifepo_current_batch batch) {
	commands_read_adc_truncate(&batch[0], P_AI_LFB1SNS, ELOC_LFB1SNS, LF_CUR_LOW, LF_CUR_HIGH, true);
	commands_read_adc_truncate(&batch[1], P_AI_LFB1OSNS, ELOC_LFB1OSNS, LF_CUR_LOW, LF_CUR_HIGH, true);
	commands_read_adc_truncate(&batch[2], P_AI_LFB2SNS, ELOC_LFB2SNS, LF_CUR_LOW, LF_CUR_HIGH, true);
	commands_read_adc_truncate(&batch[3], P_AI_LFB2OSNS, ELOC_LFB2OSNS, LF_CUR_LOW, LF_CUR_HIGH, true);
}

// TODO: make sure this all looks okay
void read_lifepo_volts_batch(lifepo_volts_batch batch) {
	read_lf_volts_precise(&(batch[0]), &(batch[1]), &(batch[2]), &(batch[3]));

	batch[0] = truncate_16t(batch[0]);
	batch[1] = truncate_16t(batch[1]);
	batch[2] = truncate_16t(batch[2]);
	batch[3] = truncate_16t(batch[3]);
}

void read_lf_volts_precise(
		uint16_t* val_1,
		uint16_t* val_2,
		uint16_t* val_3,
		uint16_t* val_4) {
	commands_read_adc(val_1, P_AI_LF1REF, ELOC_LF1REF, LF_VOLT_LOW, LF_VOLT_HIGH, true);
	commands_read_adc(val_2, P_AI_LF2REF, ELOC_LF2REF, LF_VOLT_LOW, LF_VOLT_HIGH, true);
	commands_read_adc(val_3, P_AI_LF3REF, ELOC_LF3REF, LF_VOLT_LOW, LF_VOLT_HIGH, true);
	commands_read_adc(val_4, P_AI_LF4REF, ELOC_LF4REF, LF_VOLT_LOW, LF_VOLT_HIGH, true);

// 	b1 *= 1950;
// 	b3 *= 1950;
// 	b0 = (batch[0]*3870) - batch[1];
// 	b2 = (batch[2]*3870) - batch[3];
}

void read_pdiode_batch(pdiode_batch batch) {
	for (int i = 0; i < 6; i++) {
		uint8_t rs8;
		sc = LTC1380_channel_select(PHOTO_MULTIPLEXER_I2C, i, &rs8);
		log_if_error(PD_ELOCS[i], sc, false);
		commands_read_adc_truncate(&rs8, P_AI_PD_OUT, PD_ELOCS[i], PD_LOW, PD_HIGH, false);
		batch[i] = rs8;
	}
}

void read_lifepo_temps_batch(lifepo_bank_temps_batch batch) {
	set_output(true, P_5V_EN);
	verify_5v_ref();
	for (int i = 0; i < 2; i++) {
		uint8_t rs8;
		sc = LTC1380_channel_select(TEMP_MULTIPLEXER_I2C, i, &rs8);
		log_if_error(TEMP_ELOCS[i], sc, true);
		commands_read_adc_truncate(&rs8, P_AI_TEMP_OUT, TEMP_ELOCS[i], L_TEMP_LOW, L_TEMP_HIGH, true);
		batch[i] = rs8;
	}
	set_output(false, P_5V_EN);
}

void read_lion_temps_batch(lion_temps_batch batch) {
	set_output(true, P_5V_EN);
	verify_5v_ref();
	for (int i = 2; i < 4; i++) {
		uint8_t rs8;
		sc = LTC1380_channel_select(TEMP_MULTIPLEXER_I2C, i, &rs8);
		log_if_error(TEMP_ELOCS[i], sc, true);
		commands_read_adc_truncate(&rs8, P_AI_TEMP_OUT, TEMP_ELOCS[i], L_TEMP_LOW, L_TEMP_HIGH, true);
		batch[i - 2] = rs8;
	}
	set_output(false, P_5V_EN);
}

void read_accel_batch(accelerometer_batch accel_batch) {
	uint16_t rs[3];
	sc = MPU9250_read_acc(rs);
	log_if_error(ELOC_IMU_ACC, sc, false);
	for (int i = 0; i < 3; i++) {
		accel_batch[i] = truncate_16t(rs[i]);
	}
}

void read_gyro_batch(gyro_batch gyr_batch) {
	uint16_t rs[3];
	sc = MPU9250_read_gyro(rs);
	log_if_error(ELOC_IMU_GYRO, sc, false);
	for (int i = 0; i < 3; i++) {
		log_if_out_of_bounds(rs[i], GYRO_LOW, GYRO_HIGH, ELOC_IMU_GYRO, false);
		gyr_batch[i] = truncate_16t(rs[i]);
	}
}

void read_magnetometer_batch(magnetometer_batch batch) {
	uint16_t rs[3];
	sc = MPU9250_read_mag(rs);
	log_if_error(ELOC_IMU_MAG, sc, false);
	for (int i = 0; i < 3; i++) {
		batch[i] = truncate_16t(rs[i]);
	}
}

void read_led_current_batch(led_current_batch batch) {
	commands_read_adc_truncate(&batch[0], P_AI_LED1SNS, ELOC_LED1SNS, LED_CUR_LOW, LED_CUR_HIGH, true);
	commands_read_adc_truncate(&batch[1], P_AI_LED2SNS, ELOC_LED2SNS, LED_CUR_LOW, LED_CUR_HIGH, true);
	commands_read_adc_truncate(&batch[2], P_AI_LED3SNS, ELOC_LED3SNS, LED_CUR_LOW, LED_CUR_HIGH, true);
	commands_read_adc_truncate(&batch[3], P_AI_LED4SNS, ELOC_LED4SNS, LED_CUR_LOW, LED_CUR_HIGH, true);
}

void read_radio_volts_batch(radio_volts_batch* batch) {
	uint16_t results[4];
	sc = AD7991_read_all(results, AD7991_CTRLBRD);
	log_if_error(ELOC_AD7991_1, sc, false);

	// 3V6_REF is Vin0
	batch[0] = truncate_16t(results[0]);
	log_if_out_of_bounds(results[0], RAD_VOLT_LOW, RAD_VOLT_HIGH, ELOC_AD7991_1_0, true);
	// 3V6_SNS is Vin1
	batch[1] = truncate_16t(results[1]);
	log_if_out_of_bounds(results[1], RAD_VOLT_LOW, RAD_VOLT_HIGH, ELOC_AD7991_1_1, true);
}

void read_bat_charge_volts_batch(bat_charge_volts_batch batch) {
	// analog voltages on spreadsheet
	uint16_t results[4];
	sc = AD7991_read_all(results, AD7991_CTRLBRD);
	log_if_error(ELOC_AD7991_1, sc, false);

	// 5VREF is Vin2
	batch[0] = truncate_16t(results[2]);
	log_if_out_of_bounds(results[2], CHARGE_LOW, CHARGE_HIGH, ELOC_AD7991_1_2, true);
	// 3V3REF current is Vin3
	batch[1] = truncate_16t(results[3]);
	log_if_out_of_bounds(results[3], CHARGE_LOW, CHARGE_HIGH, ELOC_AD7991_1_3, true);
}

void read_bat_charge_dig_sigs_batch(bat_charge_dig_sigs_batch* batch) {
	sc = TCA9535_init(batch);
	log_if_error(ELOC_TCA, sc, true);
}

void read_proc_temp_batch(proc_temp_batch* batch) {
	// TODO: processor temperature (likely not a signal but an atmel call)
}

void read_radio_temp_batch(radio_temp_batch* batch) {
	// TODO: Tyler will implement later
	// eventually: XDL_get_temperature(batch);
}
//
// void read_radio_current_batch(radio_current_batch* batch) {
// 	// TODO
// }

bool read_field_from_bcds(bat_charge_dig_sigs_batch batch, bcds_conversions_t shift) {
	return (batch >> shift) & 1;
}

// void read_digital_out_batch(digital_out_batch* batch) {
// 	// yet to be defined, mapped to certain events
// }

void read_imu_temp_batch(imu_temp_batch* batch) {

}
