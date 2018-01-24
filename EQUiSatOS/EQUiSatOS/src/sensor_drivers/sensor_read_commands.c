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

/************************************************************************/
/* HELPERS                                                              */
/************************************************************************/
/* NOTE: the "batch" value passed into these functions are generally arrays, so are passed by reference */

static inline uint8_t truncate_16t(uint16_t src) {
	return src >> 8;
}

static void log_if_out_of_bounds(uint reading, uint low, uint high, uint8_t eloc, bool priority) {
	if (reading <= low) {
		log_error(eloc, ECODE_READING_LOW, priority);
	} else if (reading >= high) {
		log_error(eloc, ECODE_READING_HIGH, priority);
	}
}

static void commands_read_adc_mV(uint16_t* dest, int pin, uint8_t eloc, uint low_bound, uint high_bound, bool priority) {
	sc = configure_adc(&adc_instance, pin);
	log_if_error(eloc, sc, priority);
	sc = read_adc_mV(adc_instance, dest);
	log_if_error(eloc, sc, priority);
	log_if_out_of_bounds(*dest, low_bound, high_bound, eloc, priority);
}

static void commands_read_adc_mV_truncate(uint8_t* dest, int pin, uint8_t eloc, uint low_bound, uint high_bound, bool priority) {
	uint16_t read;
	sc = configure_adc(&adc_instance, pin);
	log_if_error(eloc, sc, priority);
	sc = read_adc_mV(adc_instance, &read);
	log_if_error(eloc, sc, priority);
	log_if_out_of_bounds(read, low_bound, high_bound, eloc, priority);
	*dest = truncate_16t(read);
}

/* regulator control */

void set_5v_enable(bool on) {
	hardware_mutex_take();
	set_output(on, P_5V_EN);
	get_hw_states()->rail_5v_enabled = on;
	hardware_mutex_give();
}

void verify_regulators(void) {
	hardware_mutex_take();
	
	ad7991_ctrlbrd_batch batch;	
	read_ad7991_ctrlbrd(batch);
	struct hw_states* states = get_hw_states();
	uint16_t low3v6RefBound = states->rail_3v6_enabled ? B_3V6_REF_ON_LOW : B_3V6_REF_OFF_LOW;
	uint16_t high3v6RefBound = states->rail_3v6_enabled ? B_3V6_REF_ON_HIGH : B_3V6_REF_OFF_HIGH;
	uint16_t low3v6SnsBound = states->radio_on ? B_3V6_SNS_ON_LOW : B_3V6_SNS_OFF_LOW;
	uint16_t high3v6SnsBound = states->radio_on ? B_3V6_SNS_ON_HIGH : B_3V6_SNS_OFF_HIGH;
	uint16_t low5vRefBound = states->rail_5v_enabled ? B_5VREF_ON_LOW : B_5VREF_OFF_LOW;
	uint16_t high5vRefBound = states->rail_5v_enabled ? B_5VREF_ON_HIGH : B_5VREF_OFF_HIGH;
	
	// 3V6_REF is index 0
	log_if_out_of_bounds(batch[0], low3v6RefBound, high3v6RefBound, ELOC_AD7991_1_0, true);
	// 3V6_SNS is index 1
	log_if_out_of_bounds(batch[1], low3v6SnsBound, high3v6SnsBound, ELOC_AD7991_1_1, true);
	// 5VREF is index 2
	log_if_out_of_bounds(batch[2], low5vRefBound, high5vRefBound, ELOC_AD7991_1_2, true);
	// 3V3REF current is index 3
	log_if_out_of_bounds(batch[3], B_3V3_REF_LOW, B_3V3_REF_HIGH, ELOC_AD7991_1_3, true);
	
	hardware_mutex_give();
}

/************************************************************************/
/* SENSOR BATCH READING FUNCTIONS                                       */
/************************************************************************/

void read_ir_object_temps_batch(ir_object_temps_batch batch) {
	set_output(true, P_IR_PWR_CMD);
	vTaskDelay(IR_WAKE_DELAY);
	for (int i = 0; i < 6; i ++) {
		uint16_t obj;
		sc = MLX90614_read_all_obj(IR_ADDS[i], &obj);
		log_if_error(IR_ELOCS[i], sc, false);
		log_if_out_of_bounds(obj, B_IR_OBJ_LOW, B_IR_OBJ_HIGH, IR_ELOCS[i], false);
		batch[i] = obj;
	}
	set_output(false, P_IR_PWR_CMD);
}

void read_ir_ambient_temps_batch(ir_ambient_temps_batch batch) {
	set_output(true, P_IR_PWR_CMD);
	vTaskDelay(IR_WAKE_DELAY);
	for (int i = 0; i < 6; i++) {
		uint16_t amb;
		sc = MLX90614_read2ByteValue(IR_ADDS[i], AMBIENT, &amb);
		log_if_error(IR_ELOCS[i], sc, false);
		log_if_out_of_bounds(amb, B_IR_AMB_LOW, B_IR_AMB_HIGH, IR_ELOCS[i], false);
		batch[i] = truncate_16t(amb);
	}
	set_output(false, P_IR_PWR_CMD);
}

// TODO: make sure this all looks okay
void read_lion_volts_batch(lion_volts_batch batch) {
	uint16_t val_1_precise;
	uint16_t val_2_precise;
	
	read_li_volts_precise(&val_1_precise, &val_2_precise);
	batch[0] = truncate_16t(val_1_precise);
	batch[1] = truncate_16t(val_2_precise);
}

void read_li_volts_precise(uint16_t* val_1, uint16_t* val_2) {
	uint16_t buf;
	commands_read_adc_mV(&buf, P_AI_L1_REF, ELOC_L1_REF, B_L_VOLT_LOW, B_L_VOLT_HIGH, true);
	*val_1 = buf;

	commands_read_adc_mV(&buf, P_AI_L2_REF, ELOC_L2_REF, B_L_VOLT_LOW, B_L_VOLT_HIGH, true);
	*val_2 = buf;
}

void read_ad7991_batbrd(lion_current_batch batch1, panelref_lref_batch batch2) {
	uint16_t results[4];
	sc = AD7991_read_all_mV(results, AD7991_BATBRD);
	log_if_error(ELOC_AD7991_0, sc, true);
	
	hardware_mutex_take();
	uint16_t low_limit, high_limit;
	struct hw_states* states = get_hw_states();
	if (states->antenna_deploying || states->radio_transmitting) {
		low_limit = B_L_CUR_REG_LOW;
		high_limit = B_L_CUR_REG_HIGH;
	} else {
		low_limit = B_L_CUR_HIGH_LOW;
		high_limit = B_L_CUR_HIGH_HIGH;
	}
	// results[0] = L2_SNS
	batch1[1] = truncate_16t(results[0]);
	log_if_out_of_bounds(results[0], low_limit, high_limit, ELOC_AD7991_0_0, true);
	// results[1] = L1_SNS
	batch1[0] = truncate_16t(results[1]);
	log_if_out_of_bounds(results[1], low_limit, high_limit, ELOC_AD7991_0_1, true);
	
	// results[2] = L_REF
	batch2[1] = truncate_16t(results[2]);
	log_if_out_of_bounds(results[2], B_LREF_LOW, B_LREF_HIGH, ELOC_AD7991_0_1, true);
	// results[3] = PANELREF
	batch2[0] = truncate_16t(results[3]);
	log_if_out_of_bounds(results[3], B_PANELREF_LOW, B_PANELREF_HIGH, ELOC_AD7991_0_0, true);
	
	hardware_mutex_give();
}

void en_and_read_led_temps_batch(led_temps_batch batch) {
	set_5v_enable(true);
	verify_regulators();
	read_led_temps_batch(batch);
	set_5v_enable(false);
}

void read_led_temps_batch(led_temps_batch batch) {
	for (int i = 4; i < 8; i++) {
		uint8_t rs8;
		sc = LTC1380_channel_select(TEMP_MULTIPLEXER_I2C, i, &rs8);
		log_if_error(TEMP_ELOCS[i], sc, true);
		commands_read_adc_mV_truncate(&rs8, P_AI_TEMP_OUT, TEMP_ELOCS[i], B_LED_TEMP_LOW, B_LED_TEMP_HIGH, true);
		batch[i - 4] = rs8;
	}
}

void read_lifepo_current_batch(lifepo_current_batch batch, bool flashing_now) {
	uint low_limit, high_limit;
	if (flashing_now) {
		low_limit = B_LF_CUR_REG_LOW;
		high_limit = B_LF_CUR_REG_HIGH;
	} else {
		low_limit = B_LF_CUR_FLASH_LOW;
		high_limit = B_LF_CUR_FLASH_HIGH;
	}
	commands_read_adc_mV_truncate(&batch[0], P_AI_LFB1SNS, ELOC_LFB1SNS, low_limit, high_limit, true);
	commands_read_adc_mV_truncate(&batch[1], P_AI_LFB1OSNS, ELOC_LFB1OSNS, low_limit, high_limit, true);
	commands_read_adc_mV_truncate(&batch[2], P_AI_LFB2SNS, ELOC_LFB2SNS, low_limit, high_limit, true);
	commands_read_adc_mV_truncate(&batch[3], P_AI_LFB2OSNS, ELOC_LFB2OSNS, low_limit, high_limit, true);
}

// TODO: make sure this all looks okay
void read_lifepo_volts_batch(lifepo_volts_batch batch) {
	uint16_t val_1_precise;
	uint16_t val_2_precise;
	uint16_t val_3_precise;
	uint16_t val_4_precise;	
	
	read_lf_volts_precise(&val_1_precise, &val_2_precise, &val_3_precise, &val_4_precise);

	batch[0] = truncate_16t(val_1_precise);
	batch[1] = truncate_16t(val_2_precise);
	batch[2] = truncate_16t(val_3_precise);
	batch[3] = truncate_16t(val_4_precise);
}

void read_lf_volts_precise(uint16_t* val_1, uint16_t* val_2, uint16_t* val_3, uint16_t* val_4) {
	commands_read_adc_mV(val_1, P_AI_LF1REF, ELOC_LF1REF, B_LF_VOLT_LOW, B_LF_VOLT_HIGH, true);
	commands_read_adc_mV(val_2, P_AI_LF2REF, ELOC_LF2REF, B_LF_VOLT_LOW, B_LF_VOLT_HIGH, true);
	commands_read_adc_mV(val_3, P_AI_LF3REF, ELOC_LF3REF, B_LF_VOLT_LOW, B_LF_VOLT_HIGH, true);
	commands_read_adc_mV(val_4, P_AI_LF4REF, ELOC_LF4REF, B_LF_VOLT_LOW, B_LF_VOLT_HIGH, true);

// 	b1 *= 1950;
// 	b3 *= 1950;
// 	b0 = (batch[0]*3870) - batch[1];
// 	b2 = (batch[2]*3870) - batch[3];
}

void read_pdiode_batch(pdiode_batch batch) {
	// TODO: need to output to two bits of a uint16_t
	for (int i = 0; i < 6; i++) {
		uint16_t rs;
		// TODO: LTC1380_channel_select takes a uint8, not 16
		sc = LTC1380_channel_select(PHOTO_MULTIPLEXER_I2C, i, &rs);
		log_if_error(PD_ELOCS[i], sc, false);
		commands_read_adc_mV(&rs, P_AI_PD_OUT, PD_ELOCS[i], B_PD_LOW, B_PD_HIGH, false);
		//batch[i] = truncate_16t(rs);
	}
}

void en_and_read_lifepo_temps_batch(lifepo_bank_temps_batch batch) {
	set_5v_enable(true);
	verify_regulators();
	read_lifepo_temps_batch(batch);
	set_5v_enable(false);
}

void read_lifepo_temps_batch(lifepo_bank_temps_batch batch) {
	for (int i = 0; i < 2; i++) {
		uint8_t rs8;
		sc = LTC1380_channel_select(TEMP_MULTIPLEXER_I2C, i, &rs8);
		log_if_error(TEMP_ELOCS[i], sc, true);
		commands_read_adc_mV_truncate(&rs8, P_AI_TEMP_OUT, TEMP_ELOCS[i], B_L_TEMP_LOW, B_L_TEMP_HIGH, true);
		batch[i] = rs8;
	}
}

void en_and_read_lion_temps_batch(lion_temps_batch batch) {
	set_5v_enable(true);
	verify_regulators();
	read_lion_temps_batch(batch);
	set_5v_enable(false);
}

void read_lion_temps_batch(lion_temps_batch batch) {
	for (int i = 2; i < 4; i++) {
		uint8_t rs8;
		sc = LTC1380_channel_select(TEMP_MULTIPLEXER_I2C, i, &rs8);
		log_if_error(TEMP_ELOCS[i], sc, true);
		commands_read_adc_mV_truncate(&rs8, P_AI_TEMP_OUT, TEMP_ELOCS[i], B_L_TEMP_LOW, B_L_TEMP_HIGH, true);
		batch[i - 2] = rs8;
	}
}

void read_accel_batch(accelerometer_batch accel_batch) {
	// TODO: does IR power need to be turned back on???
	int16_t rs[3];
	sc = MPU9250_read_acc(rs);
	log_if_error(ELOC_IMU_ACC, sc, false);
	for (int i = 0; i < 3; i++) {
		accel_batch[i] = truncate_16t(rs[i]);
	}
}

void read_gyro_batch(gyro_batch gyr_batch) {
	int16_t rs[3];
	sc = MPU9250_read_gyro(rs);
	log_if_error(ELOC_IMU_GYRO, sc, false);
	for (int i = 0; i < 3; i++) {
		log_if_out_of_bounds(rs[i], B_GYRO_LOW, B_GYRO_HIGH, ELOC_IMU_GYRO, false);
		gyr_batch[i] = truncate_16t(rs[i]);
	}
}

void read_magnetometer_batch(magnetometer_batch batch) {
	int16_t rs[3];
	//sc = MPU9250_read_mag(rs);
	sc = HMC5883L_readXYZ(rs);
	log_if_error(ELOC_IMU_MAG, sc, false);
	for (int i = 0; i < 3; i++) {
		batch[i] = truncate_16t(rs[i]);
	}
}

void read_led_current_batch(led_current_batch batch, bool flashing_now) {
	uint low_limit, high_limit;
	if (flashing_now) {
		low_limit = B_LED_CUR_REG_LOW;
		high_limit = B_LED_CUR_REG_HIGH;
	} else {
		low_limit = B_LED_CUR_FLASH_LOW;
		high_limit = B_LED_CUR_FLASH_HIGH;
	}
	commands_read_adc_mV_truncate(&batch[0], P_AI_LED1SNS, ELOC_LED1SNS, low_limit, high_limit, true);
	commands_read_adc_mV_truncate(&batch[1], P_AI_LED2SNS, ELOC_LED2SNS, low_limit, high_limit, true);
	commands_read_adc_mV_truncate(&batch[2], P_AI_LED3SNS, ELOC_LED3SNS, low_limit, high_limit, true);
	commands_read_adc_mV_truncate(&batch[3], P_AI_LED4SNS, ELOC_LED4SNS, low_limit, high_limit, true);
}

/*void read_radio_volts_batch(radio_volts_batch* batch) {
	uint16_t results[4];
	sc = AD7991_read_all(results, AD7991_CTRLBRD);
	log_if_error(ELOC_AD7991_1, sc, false);

	// 3V6_REF is Vin0
	batch[0] = truncate_16t(results[0]);
	log_if_out_of_bounds(results[0], RAD_VOLT_LOW, RAD_VOLT_HIGH, ELOC_AD7991_1_0, true);
	// 3V6_SNS is Vin1
	batch[1] = truncate_16t(results[1]);
	log_if_out_of_bounds(results[1], RAD_VOLT_LOW, RAD_VOLT_HIGH, ELOC_AD7991_1_1, true);
}*/

void read_ad7991_ctrlbrd(ad7991_ctrlbrd_batch batch) {
	sc = AD7991_read_all_mV(batch, AD7991_CTRLBRD);
	log_if_error(ELOC_AD7991_1, sc, false);	
}

void read_bat_charge_dig_sigs_batch(bat_charge_dig_sigs_batch* batch) {
	sc = TCA9535_init(batch);
	log_if_error(ELOC_TCA, sc, true);
}

void read_proc_temp_batch(proc_temp_batch* batch) {
	commands_read_adc_mV_truncate(batch, ADC_POSITIVE_INPUT_TEMP, 
		ELOC_PROC_TEMP, B_PROC_TEMP_LOW, B_PROC_TEMP_HIGH, true);
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
