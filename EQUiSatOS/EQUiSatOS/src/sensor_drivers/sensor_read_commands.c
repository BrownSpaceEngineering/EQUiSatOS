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
	MLX90614_SIDEPANEL_V4_4 };

void read_ir_batch(ir_batch batch) {
	for (int i = 0; i < 6; i ++) {
		MLX90614_read2ByteValue(IRs[i] / 2, OBJ1, rs_obj1);
		MLX90614_read2ByteValue(IRs[i] / 2, OBJ2, rs_obj2);
		batch[i] = (rs_obj1.return_value + rs_obj2.return_value) / 2;
	}
}

void read_ir_temps_batch(ir_temps_batch batch) {
	for (int i = 0; i < 6; i++) {
		MLX90614_read2ByteValue(IRs[i] / 2, AMBIENT, rs_ambient);
		batch[i] = rs_ambient.return_value;
	}
}

void read_lion_volts_batch(lion_volts_batch batch) {
	// readFromADC?
	struct adc_module adc_instance1;
	struct adc_module adc_instance2;
	configure_adc(&adc_instance1, P_AI_L1_REF);
	batch[0] = read_adc(adc_instance1);
	configure_adc(&adc_instance2, P_AI_L2_REF);
	batch[1] = read_adc(adc_instance2);
}

void read_lion_current_batch(lion_current_batch batch) {
	uint16_t results[4];
	// 0 is the battery board
	AD7991_read_all(results, AD7991_ADDR_0);
	// battery 1 voltage is Vin1
	batch[0] = results[1];
	// battery 2 voltage is Vin0
	batch[1] = results[0];
}

void read_led_temps_batch(led_temps_batch batch) {
	// LTC addresses may need to be disabled
	for (int i = 0; i < 4; i++) {
		LTC1380_channel_select(TEMP_MULTIPLEXER_I2C, i, rs);
		batch[i] = rs.return_value;
	}
}

void read_lifepo_current_batch(lifepo_current_batch batch) {
	struct adc_module adc_instance1;
	struct adc_module adc_instance2;
	struct adc_module adc_instance3;
	struct adc_module adc_instance4;
	// order is bank 1, bank 1 out, bank 2, bank 2 out
	configure_adc(&adc_instance1, P_AI_LFB1SNS);
	batch[0] = read_adc(adc_instance1);
	configure_adc(&adc_instance2, P_AI_LFB1OSNS);
	batch[1] = read_adc(adc_instance2);
	configure_adc(&adc_instance3, P_AI_LFB2SNS);
	batch[2] = read_adc(adc_instance3);
	configure_adc(&adc_instance4, P_AI_LFB2OSNS);
	batch[3] = read_adc(adc_instance4);
}

void read_lifepo_volts_batch(lifepo_volts_batch batch) {
	struct adc_module adc_instance1;
	struct adc_module adc_instance2;
	struct adc_module adc_instance3;
	struct adc_module adc_instance4;
	configure_adc(&adc_instance1, P_AI_LF1REF);
	batch[0] = read_adc(adc_instance1);
	configure_adc(&adc_instance2, P_AI_LF2REF);
	batch[1] = read_adc(adc_instance2);
	configure_adc(&adc_instance3, P_AI_LF3REF);
	batch[2] = read_adc(adc_instance3);
	configure_adc(&adc_instance4, P_AI_LF4REF);
	batch[3] = read_adc(adc_instance4);
}

void read_pdiode_batch(pdiode_batch batch) {
	for (int i = 0; i < 6; i++) {
		LTC1380_channel_select(PHOTO_MULTIPLEXER_I2C, i, rs);
		batch[i] = rs.return_value;
	}
}

void read_bat_temp_batch(bat_temp_batch batch) {
	for (int i = 4; i < 8; i++) {
		LTC1380_channel_select(TEMP_MULTIPLEXER_I2C, i, rs);
		batch[i - 4] = rs.return_value;
	}
}

void read_accel_batch(accelerometer_batch accel_batch) {
	MPU9250_read_acc(&accel_batch);
}

void read_gyro_batch(gyro_batch gyr_batch) {
	MPU9250_read_gyro(&gyr_batch);
}

void read_magnetometer_batch(magnetometer_batch batch) {
	MPU9250_read_mag(&batch);
}

void read_led_current_batch(led_current_batch batch) {
	struct adc_module adc_instance1;
	struct adc_module adc_instance2;
	struct adc_module adc_instance3;
	struct adc_module adc_instance4;
	configure_adc(&adc_instance1, P_AI_LED1SNS);
	batch[0] = read_adc(adc_instance1);
	configure_adc(&adc_instance2, P_AI_LED2SNS);
	batch[1] = read_adc(adc_instance2);
	configure_adc(&adc_instance3, P_AI_LED3SNS);
	batch[2] = read_adc(adc_instance3);
	configure_adc(&adc_instance4, P_AI_LED4SNS);
	batch[3] = read_adc(adc_instance4);
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