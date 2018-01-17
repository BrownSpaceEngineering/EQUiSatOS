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

static uint16_t l_volt_1;
static uint16_t l_volt_2;
static uint16_t lf_volt_1;
static uint16_t lf_volt_2;
static uint16_t lf_volt_3;
static uint16_t lf_volt_4;
static uint32_t l_time;
static uint32_t lf_time;

/* NOTE: the "batch" value passed into these functions are generally arrays, so are passed by reference */

static uint8_t truncate_16t(uint16_t src, precise_t pt) {
	if (pt == ADC_10B) {
		return src >> 2;
	} else if (pt == ADC_12B) {
		return src >> 4;
	} else if (pt == ADC_16B) {
		return src >> 8;
	} else {
		return ~0;
	}
}

static void log_if_out_of_bounds(uint reading, uint low, uint high, uint8_t eloc, bool priority) {
	if (reading < low) {
		log_error(eloc, ECODE_READING_LOW, priority);
	} else if (reading > high) {
		log_error(eloc, ECODE_READING_HIGH, priority);
	}
}

static void commands_read_adc(uint16_t* dest, int pin, uint8_t eloc, precise_t pt, uint low_bound, uint high_bound, bool priority) {
	sc = configure_adc(&adc_instance, pin);
	log_if_error(eloc, sc, priority);
	sc = read_adc(adc_instance, dest);
	log_if_error(eloc, sc, priority);
	log_if_out_of_bounds(*dest, low_bound, high_bound, eloc, priority);
}

static void commands_read_adc_truncate(uint8_t* dest, int pin, uint8_t eloc, precise_t pt, uint low_bound, uint high_bound, bool priority) {
	uint16_t read;
	sc = configure_adc(&adc_instance, pin);
	log_if_error(eloc, sc, priority);
	sc = read_adc(adc_instance, &read);
	log_if_error(eloc, sc, priority);
	log_if_out_of_bounds(read, low_bound, high_bound, eloc, priority);
	*dest = truncate_16t(read, pt);
}

void read_ir_object_temps_batch(ir_object_temps_batch batch) {
	for (int i = 0; i < 6; i ++) {
		uint16_t obj;
		sc = MLX90614_read_all_obj(IR_ADDS[i], &obj);
		log_if_error(IR_ELOCS[i], sc, false);
		log_if_out_of_bounds(obj, IR_OBJ_LOW, IR_OBJ_HIGH, IR_ELOCS[i], false);
		batch[i] = truncate_16t(obj, ADC_16B);
	}
}

void read_ir_ambient_temps_batch(ir_ambient_temps_batch batch) {
	for (int i = 0; i < 6; i++) {
		uint16_t amb;
		sc = MLX90614_read2ByteValue(IR_ADDS[i], AMBIENT, &amb);
		log_if_error(IR_ELOCS[i], sc, false);
		log_if_out_of_bounds(amb, IR_AMB_LOW, IR_AMB_HIGH, IR_ELOCS[i], false);
		batch[i] = truncate_16t(amb, ADC_16B);
	}
}

void read_lion_volts_batch(lion_volts_batch batch) {
	uint16_t buf;
	commands_read_adc(&buf, P_AI_L1_REF, ELOC_L1_REF, ADC_10B, L_VOLT_LOW, L_VOLT_HIGH, true);
	buf *= 2500;
	l_volt_1 = buf;
	batch[0] = truncate_16t(buf, ADC_16B);
	commands_read_adc(&buf, P_AI_L2_REF, ELOC_L2_REF, ADC_10B, L_VOLT_LOW, L_VOLT_HIGH, true);
	buf *= 2500;
	l_volt_2 = buf;
	batch[1] = truncate_16t(buf, ADC_16B);

	l_time = get_current_timestamp();
}

void read_lion_current_batch(lion_current_batch batch) {
	uint16_t results[4];
	sc = AD7991_read_all(results, AD7991_BATBRD);
	log_if_error(ELOC_AD7991_0, sc, true);
	// battery 1 voltage is Vin1
	batch[0] = truncate_16t(results[1], ADC_10B);
	log_if_out_of_bounds(results[1], L_CUR_LOW, L_CUR_HIGH, ELOC_AD7991_0_1, true);
	// battery 2 voltage is Vin0
	batch[1] = truncate_16t(results[0], ADC_10B);
	log_if_out_of_bounds(results[0], L_CUR_LOW, L_CUR_HIGH, ELOC_AD7991_0_0, true);
}

void read_led_temps_batch(led_temps_batch batch) {
	for (int i = 4; i < 8; i++) {
		uint8_t rs8;
		sc = LTC1380_channel_select(TEMP_MULTIPLEXER_I2C, i, &rs8);
		log_if_error(TEMP_ELOCS[i], sc, true);
		commands_read_adc_truncate(&rs8, P_AI_TEMP_OUT, TEMP_ELOCS[i], ADC_12B, LED_TEMP_LOW, LED_TEMP_HIGH, true);
		batch[i - 4] = rs8;
	}
}

void read_lifepo_current_batch(lifepo_current_batch batch) {
	commands_read_adc_truncate(&batch[0], P_AI_LFB1SNS, ELOC_LFB1SNS, ADC_10B, LF_CUR_LOW, LF_CUR_HIGH, true);
	commands_read_adc_truncate(&batch[1], P_AI_LFB1OSNS, ELOC_LFB1OSNS, ADC_10B, LF_CUR_LOW, LF_CUR_HIGH, true);
	commands_read_adc_truncate(&batch[2], P_AI_LFB2SNS, ELOC_LFB2SNS, ADC_10B, LF_CUR_LOW, LF_CUR_HIGH, true);
	commands_read_adc_truncate(&batch[3], P_AI_LFB2OSNS, ELOC_LFB2OSNS, ADC_10B, LF_CUR_LOW, LF_CUR_HIGH, true);
}

void read_lifepo_volts_batch(lifepo_volts_batch batch) {
	uint16_t b0, b1, b2, b3;
	commands_read_adc(&b0, P_AI_LF1REF, ELOC_LF1REF, ADC_10B, LF_VOLT_LOW, LF_VOLT_HIGH, true);
	commands_read_adc(&b1, P_AI_LF2REF, ELOC_LF2REF, ADC_10B, LF_VOLT_LOW, LF_VOLT_HIGH, true);
	commands_read_adc(&b2, P_AI_LF3REF, ELOC_LF3REF, ADC_10B, LF_VOLT_LOW, LF_VOLT_HIGH, true);
	commands_read_adc(&b3, P_AI_LF4REF, ELOC_LF4REF, ADC_10B, LF_VOLT_LOW, LF_VOLT_HIGH, true);

// 	b1 *= 1950;
// 	b3 *= 1950;
// 	b0 = (batch[0]*3870) - batch[1];
// 	b2 = (batch[2]*3870) - batch[3];

	// TODO: make sure these are right
	lf_volt_1 = b0;
	lf_volt_2 = b1;
	lf_volt_3 = b2;
	lf_volt_4 = b3;
	lf_time = get_current_timestamp();

	batch[0] = truncate_16t(b0, ADC_16B);
	batch[1] = truncate_16t(b1, ADC_16B);
	batch[2] = truncate_16t(b2, ADC_16B);
	batch[3] = truncate_16t(b3, ADC_16B);
}

void read_pdiode_batch(pdiode_batch batch) {
	for (int i = 0; i < 6; i++) {
		uint8_t rs8;
		sc = LTC1380_channel_select(PHOTO_MULTIPLEXER_I2C, i, &rs8);
		log_if_error(PD_ELOCS[i], sc, false);
		commands_read_adc_truncate(&rs8, P_AI_PD_OUT, PD_ELOCS[i], ADC_12B, PD_LOW, PD_HIGH, false);
		batch[i] = rs8;
	}
}

void read_lifepo_temps_batch(lifepo_bank_temps_batch batch) {
	for (int i = 0; i < 2; i++) {
		uint8_t rs8;
		sc = LTC1380_channel_select(TEMP_MULTIPLEXER_I2C, i, &rs8);
		log_if_error(TEMP_ELOCS[i], sc, true);
		commands_read_adc_truncate(&rs8, P_AI_TEMP_OUT, TEMP_ELOCS[i], ADC_12B, L_TEMP_LOW, L_TEMP_HIGH, true);
		batch[i] = rs8;
	}
}

void read_lion_temps_batch(lion_temps_batch batch) {
	for (int i = 2; i < 4; i++) {
		uint8_t rs8;
		sc = LTC1380_channel_select(TEMP_MULTIPLEXER_I2C, i, &rs8);
		log_if_error(TEMP_ELOCS[i], sc, true);
		commands_read_adc_truncate(&rs8, P_AI_TEMP_OUT, TEMP_ELOCS[i], ADC_12B, L_TEMP_LOW, L_TEMP_HIGH, true);
		batch[i - 2] = rs8;
	}
}

void read_accel_batch(accelerometer_batch accel_batch) {
	uint16_t rs[3];
	sc = MPU9250_read_acc(rs);
	log_if_error(ELOC_IMU_ACC, sc, false);
	for (int i = 0; i < 3; i++) {
		accel_batch[i] = truncate_16t(rs[i], ADC_16B);
	}
}

void read_gyro_batch(gyro_batch gyr_batch) {
	uint16_t rs[3];
	sc = MPU9250_read_gyro(rs);
	log_if_error(ELOC_IMU_GYRO, sc, false);
	for (int i = 0; i < 3; i++) {
		log_if_out_of_bounds(rs[i], GYRO_LOW, GYRO_HIGH, ELOC_IMU_GYRO, false);
		gyr_batch[i] = truncate_16t(rs[i], ADC_16B);
	}
}

void read_magnetometer_batch(magnetometer_batch batch) {
	uint16_t rs[3];
	sc = MPU9250_read_mag(rs);
	log_if_error(ELOC_IMU_MAG, sc, false);
	for (int i = 0; i < 3; i++) {
		batch[i] = truncate_16t(rs[i], ADC_16B);
	}
}

void read_led_current_batch(led_current_batch batch) {
	commands_read_adc_truncate(&batch[0], P_AI_LED1SNS, ELOC_LED1SNS, ADC_10B, LED_CUR_LOW, LED_CUR_HIGH, true);
	commands_read_adc_truncate(&batch[1], P_AI_LED2SNS, ELOC_LED2SNS, ADC_10B, LED_CUR_LOW, LED_CUR_HIGH, true);
	commands_read_adc_truncate(&batch[2], P_AI_LED3SNS, ELOC_LED3SNS, ADC_10B, LED_CUR_LOW, LED_CUR_HIGH, true);
	commands_read_adc_truncate(&batch[3], P_AI_LED4SNS, ELOC_LED4SNS, ADC_10B, LED_CUR_LOW, LED_CUR_HIGH, true);
}

void read_radio_volts_batch(radio_volts_batch* batch) {
	// 3v6_ref and 3v6_sns
	uint16_t results[4];
	sc = AD7991_read_all(results, AD7991_CTRLBRD);
	log_if_error(ELOC_AD7991_1, sc, false);

	// 3V6 voltage is Vin0
	batch[0] = truncate_16t(results[0], ADC_10B);
	log_if_out_of_bounds(results[0], RAD_VOLT_LOW, RAD_VOLT_HIGH, ELOC_AD7991_1_0, true);
	// 3V6 current is Vin1
	batch[1] = truncate_16t(results[1], ADC_10B);
	log_if_out_of_bounds(results[1], RAD_VOLT_LOW, RAD_VOLT_HIGH, ELOC_AD7991_1_1, true);
}

void read_bat_charge_volts_batch(bat_charge_volts_batch batch) {
	// analog voltages on spreadsheet
	uint16_t results[4];
	sc = AD7991_read_all(results, AD7991_CTRLBRD);
	log_if_error(ELOC_AD7991_1, sc, false);

	// 5V voltage is Vin2
	batch[0] = truncate_16t(results[2], ADC_10B);
	log_if_out_of_bounds(results[2], CHARGE_LOW, CHARGE_HIGH, ELOC_AD7991_1_2, true);
	// 3V3 current is Vin3
	batch[1] = truncate_16t(results[3], ADC_10B);
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

void get_current_lion_volts(
		uint16_t* val_1,
		uint16_t* val_2,
		uint32_t* timestamp) {
	*val_1 = l_volt_1;
	*val_2 = l_volt_2;
	*timestamp = l_time;
}

void get_current_lifepo_volts(
		uint16_t* val_1,
		uint16_t* val_2,
		uint16_t* val_3,
		uint16_t* val_4,
		uint32_t* timestamp) {
	*val_1 = lf_volt_1;
	*val_2 = lf_volt_2;
	*val_3 = lf_volt_3;
	*val_4 = lf_volt_4;
	*timestamp = lf_time;
}
