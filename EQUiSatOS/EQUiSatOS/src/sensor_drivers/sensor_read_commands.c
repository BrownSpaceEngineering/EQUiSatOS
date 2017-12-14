/*
 * sensor_read_commands.c
 *
 * Created: 11/1/2016 8:18:16 PM
 *  Author: mckenna
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
	return src >>> uint8_t;
}

static void commands_read_adc(uint8_t* dest, uint8_t pin, uint8_t eloc, bool priority) {
	uint16_t read;
	sc = configure_adc(&adc_instance, pin);
	log_if_error(eloc, sc, priority);
	sc = read_adc(adc_instance, &read);
	log_if_error(eloc, sc, priority);
	*dest = truncate_16t(read);
}

static void log_if_out_of_bounds(uint reading, uint low, uint high, uint8_t eloc, bool priority) {
	if (reading < low) {
		log_error(eloc, ECODE_READING_LOW, priority);
	} else if (reading > high) {
		log_error(eloc, ECODE_READING_HIGH, priority);
	}
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
		batch[i] = amb;
	}
}

void read_lion_volts_batch(lion_volts_batch batch) {
	commands_read_adc(&batch[0], P_AI_L1_REF, ELOC_L1_REF, true);
	batch[0] *= 2500;
	log_if_out_of_bounds(batch[0], L_VOLT_LOW, L_VOLT_HIGH, P_AI_L1_REF, true);
	commands_read_adc(&batch[1], P_AI_L2_REF, ELOC_L2_REF, true);
	batch[1] *= 2500;
	log_if_out_of_bounds(batch[1], L_VOLT_LOW, L_VOLT_HIGH, P_AI_L2_REF, true);
}

void read_lion_current_batch(lion_current_batch batch) {
	uint16_t results[4];	
	sc = AD7991_read_all(results, AD7991_BATBRD);
	log_if_error(ELOC_AD7991_0, sc, true);
	// battery 1 voltage is Vin1
	batch[0] = results[1];
	log_if_out_of_bounds(batch[0], L_CUR_LOW, L_CUR_HIGH, ELOC_AD7991_0_1, true);
	// battery 2 voltage is Vin0
	batch[1] = results[0];
	log_if_out_of_bounds(batch[1], L_CUR_LOW, L_CUR_HIGH, ELOC_AD7991_0_0, true);
}

void read_led_temps_batch(led_temps_batch batch) {
	for (int i = 4; i < 8; i++) {
		uint8_t rs8;
		uint16_t rs;
		sc = LTC1380_channel_select(TEMP_MULTIPLEXER_I2C, i, &rs8);
		log_if_error(TEMP_ELOCS[i], sc, true);
		commands_read_adc(&rs, P_AI_TEMP_OUT, TEMP_ELOCS[i], true);
		log_if_out_of_bounds(rs, LED_TEMP_LOW, LED_TEMP_HIGH, TEMP_ELOCS[i], true);
		batch[i - 4] = rs;
	}
}

void read_lifepo_current_batch(lifepo_current_batch batch) {
	commands_read_adc(&batch[0], P_AI_LFB1SNS, ELOC_LFB1SNS, true);
	commands_read_adc(&batch[1], P_AI_LFB1OSNS, ELOC_LFB1OSNS, true);
	commands_read_adc(&batch[2], P_AI_LFB2SNS, ELOC_LFB2SNS, true);
	commands_read_adc(&batch[3], P_AI_LFB2OSNS, ELOC_LFB2OSNS, true);

	log_if_out_of_bounds(batch[0], LF_CUR_LOW, LF_CUR_HIGH, ELOC_LFB1SNS, true);
	log_if_out_of_bounds(batch[1], LF_CUR_LOW, LF_CUR_HIGH, ELOC_LFB1OSNS, true);
	log_if_out_of_bounds(batch[2], LF_CUR_LOW, LF_CUR_HIGH, ELOC_LFB2SNS, true);
	log_if_out_of_bounds(batch[3], LF_CUR_LOW, LF_CUR_HIGH, ELOC_LFB2OSNS, true);
}

void read_lifepo_volts_batch(lifepo_volts_batch batch) {
	commands_read_adc(&batch[0], P_AI_LF1REF, ELOC_LF1REF, true);
	commands_read_adc(&batch[1], P_AI_LF2REF, ELOC_LF2REF, true);
	commands_read_adc(&batch[2], P_AI_LF3REF, ELOC_LF3REF, true);
	commands_read_adc(&batch[3], P_AI_LF4REF, ELOC_LF4REF, true);

// 	batch[1] *= 1950;
// 	batch[3] *= 1950;
// 	batch[0] = (batch[0]*3870) - batch[1];
// 	batch[2] = (batch[2]*3870) - batch[3];
	log_if_out_of_bounds(batch[0], LF_VOLT_LOW, LF_VOLT_HIGH, ELOC_LF1REF, true);
	log_if_out_of_bounds(batch[1], LF_VOLT_LOW, LF_VOLT_HIGH, ELOC_LF2REF, true);
	log_if_out_of_bounds(batch[2], LF_VOLT_LOW, LF_VOLT_HIGH, ELOC_LF3REF, true);
	log_if_out_of_bounds(batch[3], LF_VOLT_LOW, LF_VOLT_HIGH, ELOC_LF4REF, true);
}

void read_pdiode_batch(pdiode_batch batch) {
	for (int i = 0; i < 6; i++) {
		uint8_t rs8;
		uint16_t rs;
		sc = LTC1380_channel_select(PHOTO_MULTIPLEXER_I2C, i, &rs8);
		log_if_error(PD_ELOCS[i], sc, false);
		commands_read_adc(&rs, P_AI_PD_OUT, PD_ELOCS[i], false);
		log_if_out_of_bounds(rs, PD_LOW, PD_HIGH, PD_ELOCS[i], false);
		batch[i] = rs;
	}
}

void read_lifepo_temps_batch(lifepo_bank_temps_batch batch) {
	for (int i = 0; i < 2; i++) {
		uint8_t rs8;
		uint16_t rs;
		sc = LTC1380_channel_select(TEMP_MULTIPLEXER_I2C, i, &rs8);
		log_if_error(TEMP_ELOCS[i], sc, true);
		commands_read_adc(&rs, P_AI_TEMP_OUT, TEMP_ELOCS[i], true);
		log_if_out_of_bounds(rs, L_TEMP_LOW, L_TEMP_HIGH, TEMP_ELOCS[i], true);
		batch[i] = rs;
	}
}

void read_lion_temps_batch(lion_temps_batch batch) {
	for (int i = 2; i < 4; i++) {
		uint8_t rs8;
		uint16_t rs;
		sc = LTC1380_channel_select(TEMP_MULTIPLEXER_I2C, i, &rs8);
		log_if_error(TEMP_ELOCS[i], sc, true);
		commands_read_adc(&rs, P_AI_TEMP_OUT, TEMP_ELOCS[i], true);
		log_if_out_of_bounds(rs, L_TEMP_LOW, L_TEMP_HIGH, TEMP_ELOCS[i], true);
		batch[i - 2] = rs;
	}
}

void read_accel_batch(accelerometer_batch accel_batch) {
	sc = MPU9250_read_acc((uint16_t*) accel_batch);
	log_if_error(ELOC_IMU_ACC, sc, false);
}

void read_gyro_batch(gyro_batch gyr_batch) {
	sc = MPU9250_read_gyro((uint16_t*) gyr_batch);
	log_if_error(ELOC_IMU_GYRO, sc, false);
	for (int i = 0; i < 3; i++) {
		log_if_out_of_bounds(gyr_batch[i], GYRO_LOW, GYRO_HIGH, ELOC_IMU_GYRO, false);
	}
}

void read_magnetometer_batch(magnetometer_batch batch) {
	//sc = MPU9250_read_mag((uint16_t*) batch);
	//log_if_error(ELOC_IMU_MAG, sc, false);
}

void read_led_current_batch(led_current_batch batch) {
	commands_read_adc(&batch[0], P_AI_LED1SNS, ELOC_LED1SNS, true);
	commands_read_adc(&batch[1], P_AI_LED2SNS, ELOC_LED2SNS, true);
	commands_read_adc(&batch[2], P_AI_LED3SNS, ELOC_LED3SNS, true);
	commands_read_adc(&batch[3], P_AI_LED4SNS, ELOC_LED4SNS, true);

	log_if_out_of_bounds(batch[0], LED_CUR_LOW, LED_CUR_HIGH, ELOC_LED1SNS, true);
	log_if_out_of_bounds(batch[1], LED_CUR_LOW, LED_CUR_HIGH, ELOC_LED2SNS, true);
	log_if_out_of_bounds(batch[2], LED_CUR_LOW, LED_CUR_HIGH, ELOC_LED3SNS, true);
	log_if_out_of_bounds(batch[3], LED_CUR_LOW, LED_CUR_HIGH, ELOC_LED4SNS, true);
}

void read_radio_volts_batch(radio_volts_batch* batch) {
	// 3v6_ref and 3v6_sns
	uint16_t results[4];	
	sc = AD7991_read_all(results, AD7991_CTRLBRD);
	log_if_error(ELOC_AD7991_1, sc, false);
	
	
	
	// TODO: I thought we had only one radio temperature... what are the two here? - mckenna
	
	
	
	
	// 3V6 voltage is Vin0
	batch[0] = results[0];
	log_if_out_of_bounds(batch[0], RAD_VOLT_LOW, RAD_VOLT_HIGH, ELOC_AD7991_1_0, true);
	// 3V6 current is Vin1
	batch[1] = results[1];
	log_if_out_of_bounds(batch[1], RAD_VOLT_LOW, RAD_VOLT_HIGH, ELOC_AD7991_1_1, true);
}

void read_bat_charge_volts_batch(bat_charge_volts_batch batch) {
	// analog voltages on spreadsheet
	uint16_t results[4];	
	sc = AD7991_read_all(results, AD7991_CTRLBRD);
	log_if_error(ELOC_AD7991_1, sc, false);
	// 5V voltage is Vin2
	batch[0] = results[2];
	log_if_out_of_bounds(batch[0], CHARGE_LOW, CHARGE_HIGH, ELOC_AD7991_1_2, true);
	// 3V3 current is Vin3
	batch[1] = results[3];
	log_if_out_of_bounds(batch[1], CHARGE_LOW, CHARGE_HIGH, ELOC_AD7991_1_3, true);
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
