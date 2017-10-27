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

read_ir_batch(ir_batch batch) {
	for (int i = 0; i < 12; i += 2) {
		return_struct_16 rs_obj1;
		return_struct_16 rs_obj2;
		return_struct_16 rs_ambient;
		MLX90614_read2ByteValue(IRs[i] / 2, OBJ1, rs_obj1);
		MLX90614_read2ByteValue(IRs[i] / 2, OBJ2, rs_obj2);
		MLX90614_read2ByteValue(IRs[i] / 2, AMBIENT, rs_ambient);
		batch[i] = (rs_obj1.return_value + rs_obj2.return_value) / 2;
		batch[i + 1] = rs_ambient.return_value;
	}
}

read_lion_volts_batch(lion_volts_batch batch) {
	struct adc_module adc_instance;
	configure_adc(&adc_instance, P_AI_L1_REF);
	batch[0] = read_adc(adc_instance);
	configure_adc(&adc_instance, P_AI_L2_REF);
	batch[1] = read_adc(adc_instance);
}

read_lion_current_batch(lion_current_batch batch) {
	// not really sure what to do here with the new AD7991_ADC commands
}

read_led_temps_batch(led_temps_batch batch) {
	
}

read_lifepo_current_batch(lifepo_current_batch batch) {
	// order is bank 1, bank 1 out, bank 2, bank 2 out
	struct adc_module adc_instance;
	configure_adc(&adc_instance, P_AI_LFB1SNS);
	batch[0] = read_adc(adc_instance);
	configure_adc(&adc_instance, P_AI_LFB1OSNS);
	batch[1] = read_adc(adc_instance);
	configure_adc(&adc_instance, P_AI_LFB2SNS);
	batch[2] = read_adc(adc_instance);
	configure_adc(&adc_instance, P_AI_LFB2OSNS);
	batch[3] = read_adc(adc_instance);
}

read_lifepo_volts_batch(lifepo_volts_batch batch) {
	struct adc_module adc_instance;
	configure_adc(&adc_instance, P_AI_LF1REF);
	batch[0] = read_adc(adc_instance);
	configure_adc(&adc_instance, P_AI_LF2REF);
	batch[1] = read_adc(adc_instance);
	configure_adc(&adc_instance, P_AI_LF3REF);
	batch[2] = read_adc(adc_instance);
	configure_adc(&adc_instance, P_AI_LF4REF);
	batch[3] = read_adc(adc_instance);
}

read_diode_batch(diode_batch batch) {
	
}

read_bat_temp_batch(bat_temp_batch batch) {
	
}

read_ir_temps_batch(ir_temps_batch batch) {
	
}

read_radio_temp_batch(radio_temp_batch batch) {
	
}

read_imu_batch(accelerometer_batch accel_batch, gyro_batch gyr_batch) {
	// arrays are entered as x, y, z
	MPU9250Reading accel, gyro;
	
	MPU9250_read_acc(&accel);
	MPU9250_read_gyro(&gyro);
	ThreeAxisReading ac = accel.accel;
	ThreeAxisReading gy = gyro.gyro;
	
	accel_batch[0] = ac.x;
	accel_batch[1] = ac.y;
	accel_batch[2] = ac.z;
	gyr_batch[0] = gy.x;
	gyr_batch[1] = gy.y;
	gyr_batch[2] = gy.z;
}

read_magnetometer_batch(magnetometer_batch batch) {
	// array is entered as x, y, z
	MPU9250Reading mag;
	MPU9250_read_mag(&mag);
	ThreeAxisReading m = mag.mag;
	
	batch[0] = m.x;
	batch[1] = m.y;
	batch[2] = m.z;
}

read_led_current_batch(led_current_batch batch) {
	struct adc_module adc_instance;
	configure_adc(&adc_instance, P_AI_LED1SNS);
	batch[0] = read_adc(adc_instance);
	configure_adc(&adc_instance, P_AI_LED2SNS);
	batch[1] = read_adc(adc_instance);
	configure_adc(&adc_instance, P_AI_LED3SNS);
	batch[2] = read_adc(adc_instance);
	configure_adc(&adc_instance, P_AI_LED4SNS);
	batch[3] = read_adc(adc_instance);
}
	
read_radio_volts_batch(radio_volts_batch batch) {
	
}

read_bat_charge_volts_batch(bat_charge_volts_batch batch) {
	
}

read_bat_charge_dig_sigs_batch(bat_charge_dig_sigs_batch batch) {
	
}
	
read_digital_out_batch(digital_out_batch batch) {
		
}