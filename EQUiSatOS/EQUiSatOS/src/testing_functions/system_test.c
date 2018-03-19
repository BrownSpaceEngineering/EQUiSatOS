#include "system_test.h"

static void set_regulator_power(bool on) {
	// bypass RTOS
	set_output(on, P_RAD_PWR_RUN);
	set_output(on, P_RAD_SHDN);
	set_output(on, P_5V_EN);
}

static uint16_t tca_shifts(uint16_t batch, bool i1, bool i2, bool i3, bool i4, bool i5, bool i6) {
	// zero out the places we're going to overwrite
	// see order in Message Format spreadsheet
	batch &= 0xF3F0;
	// fill in the new values we want
	batch |= i1;
	batch |= (i2<<1);
	batch |= (i3<<2);
	batch |= (i4<<3);
	batch |= (i5<<10);
	batch |= (i6<<11);
	
	return batch;
}

void tca_shifts_test(void) {
	uint16_t batch = 0;
	assert(0 == tca_shifts(batch, 0, 0, 0, 0, 0, 0));
	assert(0x000F == tca_shifts(batch, 1, 1, 1, 1, 0, 0));
	assert(0x0C00 == tca_shifts(batch, 0, 0, 0, 0, 1, 1));
	assert(0x0C0F == tca_shifts(batch, 1, 1, 1, 1, 1, 1));
	
	batch = ~0;
	assert(0xF3F0 == tca_shifts(batch, 0, 0, 0, 0, 0, 0));
	assert(0xFFF0 == tca_shifts(batch, 0, 0, 0, 0, 1, 1));
	assert(0xFFFF == tca_shifts(batch, 1, 1, 1, 1, 1, 1));
}

void sensor_read_tests(void) {
	uint16_t six_buf_16t[6];
	uint8_t six_buf[6];
	uint8_t three_buf[3];
	uint8_t four_buf[4];
	uint16_t four_buf_16t[4];
	uint8_t two_buf[2];	
	uint16_t pd;
	delay_ms(100);
	print("\n\n\n\n##### NEW RUN #####\n");
	
	print("\n# IMU #\n");
	read_accel_batch(three_buf);
	print_accel_batch(three_buf);
	read_gyro_batch(three_buf);
	print_gyro_batch(three_buf);
	read_magnetometer_batch(three_buf);
	print_magnetometer_batch(three_buf);

	print("\n# IR #\n");
	read_ir_ambient_temps_batch(six_buf);
	print_ir_ambient_temps_batch(six_buf);
	read_ir_object_temps_batch(six_buf_16t);
	print_ir_object_temps_batch(six_buf_16t);

	print("\n# PDIODE #\n");
	read_pdiode_batch(&pd);
	print_pdiode_batch(pd);

	print("\n\n# LiON VOLTS #\n");
	read_lion_volts_batch(two_buf);
	print_lion_volts_batch(two_buf);
	
	print("# ad7991_batbrd #\n");
	read_ad7991_batbrd(four_buf, four_buf+2);	
	print("L1_SNS: %d, L2_SNS %d, PANELREF %d, LREF %d\n", (uint16_t)four_buf[0]<<8, (uint16_t)four_buf[1]<<8, (uint16_t)four_buf[2]<<8, (uint16_t)four_buf[3]<<8);
	
	set_regulator_power(false);
	print("# ad7991_ctrlbrd regs OFF#\n");
	read_ad7991_ctrlbrd(four_buf_16t);
	print("3V6REF: %d, 3V6_SNS %d, 5Vref %d, 3V3REF %d\n", four_buf_16t[0], four_buf_16t[1], four_buf_16t[2], four_buf_16t[3]);	
	set_regulator_power(true);
	delay_ms(1000);
	print("# ad7991_ctrlbrd regs ON#\n");
	read_ad7991_ctrlbrd(four_buf_16t);
	print("3V6REF: %d, 3V6_SNS %d, 5Vref %d, 3V3REF %d\n", four_buf_16t[0], four_buf_16t[1], four_buf_16t[2], four_buf_16t[3]);
	set_regulator_power(false);

	print("# LiON TEMPS #\n");
	en_and_read_lion_temps_batch(two_buf);
	print_lion_temps_batch(two_buf);

	print("\n# LiFePO VOLTS #\n");
	read_lifepo_volts_batch(four_buf);
	print_lifepo_volts_batch(four_buf);

	print("# LiFePO CURRENT #\n");
	_read_lifepo_current_batch_unsafe(four_buf, false);
	print_lifepo_current_batch(four_buf);
	
	print("# LED CURRENT #\n");
	_read_led_current_batch_unsafe(four_buf, false);
	print_led_current_batch(four_buf);
	
	
}

// Function that takes in a status code and prints out the matching status
static void get_status(enum status_code code, char * buffer){
	switch(code){

		case STATUS_OK:
		strcpy(buffer,"Status OK");
		break;

		case STATUS_ERR_BAD_ADDRESS:
		strcpy(buffer,"Status error, bad address");
		break;

		case STATUS_ERR_BAD_DATA:
		strcpy(buffer,"Status error, bad data");
		break;

		case STATUS_ERR_BAD_FORMAT:
		strcpy(buffer,"Status error, bad format");
		break;

		case STATUS_BUSY:
		strcpy(buffer,"Status busy");
		break;

		case STATUS_ERR_PACKET_COLLISION:
		strcpy(buffer,"Status error, packet collision");
		break;

		default:
		strcpy(buffer,"Status unknown error");
	}

}

// function that take in a panel address (hex) and puts the name of that pannel into buffer
static void get_ir_panel(int panel_addr, char* buffer){

	switch (panel_addr){

		case IR_POS_Y:
		strcpy(buffer,"IR_POS_Y");
		break;

		case IR_POS_Z:
		strcpy(buffer,"IR_POS_Z");
		break;

		case IR_POS_X:
		strcpy(buffer,"IR_POS_X");
		break;

		case IR_NEG_Z:
		strcpy(buffer,"IR_NEG_Z");
		break;

		case IR_NEG_X:
		strcpy(buffer,"IR_NEG_X");
		break;

		case IR_NEG_Y:
		strcpy(buffer,"IR_NEG_Y");
		break;
		
		default:
		strcpy(buffer,"Unknown IR Address");
	}
}

// test all addresses in the array (addr) and fill results in (results) specify number of address input with (num)
static void MLX90614_test(bool printFloats){
	setup_pin(true, P_IR_PWR_CMD);
	set_output(true, P_IR_PWR_CMD);
	print("==============MLX90614 Test==============\n");
	print("Panel Name \t Obj Status \t Obj \t Amb Status \t Amb \n");

	//read the value from the sensor specified by (addr) and put value in rs
	uint16_t buf;
	char buffer[40];
	enum status_code sc;
	uint8_t addr[] = {IR_POS_Z, IR_NEG_X, IR_NEG_Y, IR_POS_Y, IR_NEG_Z, IR_POS_X};

	for (int i = 0; i < 6; i++){
		sc = MLX90614_read_all_obj(addr[i],&buf);

		get_ir_panel(addr[i],buffer); // get panel address string
		print("%s: \t ",buffer);
		get_status(sc,buffer); // get error code string
		if (printFloats) {
			print("%s \t %.02f\t",buffer,dataToTemp(buf));
		} else {
			print("%s \t %d\t",buffer,(uint16_t) dataToTemp(buf));
		}	

		sc = MLX90614_read_amb(addr[i],&buf);
		get_status(sc,buffer);
		if (printFloats) {
			print("%s \t %.02f\n",buffer,dataToTemp(buf));	
		} else {
			print("%s \t %d\n",buffer,(uint16_t)dataToTemp(buf));
		}
	}
}


static void AD590_test(void){
	char test_str[20];
	char error_str[20];	
	struct adc_module temp_instance; //generate object	
	
	set_output(true, P_5V_EN);
	
	print("==============AD590 Test==============\n");
	for (int i = 0; i < 8; i++){
		configure_adc(&temp_instance,P_AI_TEMP_OUT, true);
		uint8_t rs;
		enum status_code sc = LTC1380_channel_select(TEMP_MULTIPLEXER_I2C, i, &rs);		
		
		delay_ms(1);

		uint16_t temp_mV;
		read_adc_mV(temp_instance,&temp_mV);
		
		// temperature conversion from voltage -> current -> degrees celsius		
		//float current = ((float)temp_mV)/1000/2197. -0.000153704; //converts from V to A
		//float tempInC = (current)*1000000-273;// T = 454*V in C
		float tempInC = ((float) temp_mV) *0.1286 - 107.405;
		
		get_status(sc,error_str);	
		switch (i) {
			case 0:
				strcpy(test_str,"LF3_TEMP");
				break;
			case 1:
				strcpy(test_str,"LF1_TEMP");
				break;
			case 2:
				strcpy(test_str,"L2_TEMP");
				break;
			case 3:
				strcpy(test_str,"L1_TEMP");
				break;
			case 4:
				strcpy(test_str,"LED4TEMP");
				break;
			case 5:
				strcpy(test_str,"LED3TEMP");
				break;
			case 6:
				strcpy(test_str,"LED2TEMP");
				break;
			case 7:
				strcpy(test_str,"LED1TEMP");
				break;						
		}
		print(" %s \t %s \t %d mV \t %d C\n",test_str,error_str,temp_mV, (int)tempInC);		
	}

	// compare_results((void *) temps,(void *) expected,4, 5, "AD590");		
}

//IMU test if rebias = 0 then don't compute the bias again
// input length 3 array (sensors) each array entry corresponds to a sensor, 0 means don't read, else read
// sensors[0] -> acc, sensors[1] -> gyro, sensors[3] -> mag  (length of toFill should correspond to sensor reads)
static void MPU9250_test(bool rebias, bool printFloats){
	print("==============MPU9250 Test==============\n");	
	// IMU
	// testmap 0 - acc x : 1 - acc y : 2 - acc z : 3 - gyr x : 4 - gyr y : 5 - gyr z

	// re compute bias on call
	if (rebias) {
		uint16_t gyroBias[3] = {0, 0, 0}, accelBias[3] = {0, 0, 0};
		MPU9250_computeBias(gyroBias,accelBias);
	}

	int16_t MPU9250_results[3];
	char buffer[20];
	enum status_code code;
	
	code = MPU9250_read_acc_EQUiSat_coords(MPU9250_results);
	get_status(code,buffer);
	if (printFloats) {
		print("ACCELEROMETER: \t %s \n x: \t %d \t %.02f g \n y: \t %d \t %.02f g \n z: \t %d \t %.02f g\n",buffer,MPU9250_results[0], (float)MPU9250_results[0]/16384.,MPU9250_results[1], (float)MPU9250_results[1]/16384., MPU9250_results[2], (float)MPU9250_results[2]/16384.);
	} else {
		print("ACCELEROMETER: \t %s \n x: \t %d \t %d g \n y: \t %d \t %d g \n z: \t %d \t %d g\n",buffer,MPU9250_results[0], MPU9250_results[0]/16384,MPU9250_results[1], MPU9250_results[1]/16384, MPU9250_results[2], MPU9250_results[2]/16384);
	}			
	
	code = MPU9250_read_gyro_EQUiSat_coords(MPU9250_results);		
	get_status(code,buffer);
	if (printFloats) {
		print("GYRO: \t %s \n x: \t %d \t %.02f d/s \n y: \t %d \t %.02f d/s \n z: \t %d \t %.02f d/s\n",buffer,MPU9250_results[0], (float)MPU9250_results[0]/131.,MPU9250_results[1], (float)MPU9250_results[1]/131., MPU9250_results[2], (float)MPU9250_results[2]/131.);
		} else {
		print("GYRO: \t %s \n x: \t %d \t %d d/s \n y: \t %d \t %d d/s \n z: \t %d \t %d d/s\n",buffer,MPU9250_results[0], MPU9250_results[0]/131,MPU9250_results[1], MPU9250_results[1]/131, MPU9250_results[2], MPU9250_results[2]/131);
	}
	
		
	code = MPU9250_read_mag_EQUiSat_coords(MPU9250_results);
	get_status(code, buffer);
	if (printFloats) {
		print("MAG: \t %s \n x: \t %d \t %.02f uT \n y: \t %d \t %.02f uT \n z: \t %d \t %.02f uT\n",buffer,MPU9250_results[0], (float)MPU9250_results[0]*0.6,MPU9250_results[1], (float)MPU9250_results[1]*0.6, MPU9250_results[2], (float)MPU9250_results[2]*0.6);
		} else {
		print("MAG: \t %s \n x: \t %d \t %d uT \n y: \t %d \t %d uT \n z: \t %d \t %d uT\n",buffer,MPU9250_results[0], MPU9250_results[0]*3/5,MPU9250_results[1], MPU9250_results[1]*3/5, MPU9250_results[2], MPU9250_results[2]*3/5);
	}
	
	//uint16_t MPU9250_expected[] = {0, 0, 0, 0, 0, 0};
	//if (MPU9250_results[i] > MPU9250_expected[i]){
	//if((MPU9250_results[i] - MPU9250_expected[i]) >= MPU9250_err_margin) {
	//print("Error in test MPU9250 number %d \n",i);
	//pass = 0;
	//}
	//} else {
	//if((MPU9250_results[i] - MPU9250_expected[i]) >= MPU9250_err_margin) {
	//print("Error in test MPU9250 number %d \n",i);
	//pass = 0;
	//}	
}

//HMC5883L Magnetometer test
static void HMC5883L_test(bool printFloats){
	print("==============HMC5883L Test==============\n");
	char buffer[20];
	int16_t xyz[3];	
	enum status_code code = HMC5883L_readXYZ(xyz);
	get_status(code, buffer);
	if (printFloats) {
		print("MAG: \t %s \n x: \t %d \t %.04f G \n y: \t %d \t %.04f G \n z: \t %d \t %.04f G\n",buffer,xyz[0], (float)xyz[0]/1370,xyz[1], (float)xyz[1]/1370, xyz[2], (float)xyz[2]/1370);
		} else {
		print("MAG: \t %s \n x: \t %d \t %d mG \n y: \t %d \t %d mG \n z: \t %d \t %d mG\n",buffer,xyz[0], xyz[0]*1000/1370,xyz[1], xyz[1]*1000/1370, xyz[2], xyz[2]*1000/1370);
	}

}

//GPIO test
static void TCA9535_test(bool printStData, bool printChargeData){
	if (printStData && printChargeData) {
		print("==============TCA9535 Test==============\n");
	}
	uint16_t res;
	enum status_code sc = TCA9535_init(&res);
	print("TCA return status: ");
	print_error(sc);
	if (printStData) {
		print("L2_ST: \t\t %d\n", (res>>8)&0x1);
		print("L1_ST: \t\t %d\n", (res>>9)&0x1);
		print("SPF_ST: \t %d\n", (res>>10)&0x1);	
	}
	if (printChargeData) {
		print("L1_CHGN: \t %d\n", (res>>12)&0x1);
		print("L1_FAULTN: \t %d\n", (res>>13)&0x1);
		print("L2_CHGN: \t %d\n", (res>>14)&0x1);
		print("L2_FAULTN: \t %d\n", (res>>15)&0x1);
		print("LF_B1_CHGN: \t %d\n", (res>>7)&0x1);
		print("LF_B1_FAULTN: \t %d\n", (res>>6)&0x1);
		print("LF_B2_CHGN: \t %d\n", (res>>4)&0x1);
		print("LF_B2_FAULTN: \t %d\n", (res>>5)&0x1);	
	}	
}

// Photodiode test
static void TEMD6200_test(void){
	print("==============TEMD6200 Test==============\n");
	char buffer[20];	
	char test_str[20];
	struct adc_module pd_instance;
	for (int i = 0; i < 6; i++){		
		uint16_t pd_mV;
		configure_adc(&pd_instance,P_AI_PD_OUT, true);
		uint8_t rs;		
		enum status_code code = LTC1380_channel_select(0x4a, i, &rs);
		get_status(code, buffer);
		adc_enable(&pd_instance);
		read_adc_mV(pd_instance, &pd_mV);
		switch (i) {
			case 0:
			strcpy(test_str,"PD_POS_Z");
			break;
			case 1:
			strcpy(test_str,"PD_NEG_X");
			break;
			case 2:
			strcpy(test_str,"PD_NEG_Y");
			break;
			case 3:
			strcpy(test_str,"PD_POS_Y");
			break;
			case 4:
			strcpy(test_str,"PD_NEG_Z");
			break;
			case 5:
			strcpy(test_str,"PD_POS_X");
			break;
		}
		print("%s \t %s \t %d mV \t %d \n",test_str, buffer, pd_mV, get_pdiode_two_bit_range(pd_mV));
	}
	
	//pdBuffer[i] =(readVoltagemV(pd_instance));//-6.5105)/0.3708; // I = exp((V-6.5105/0.3708)) in uA	
}

static void AD7991_BAT_test(void){

	print("==============AD7991_BATBRD Test==============\n");
	char test_str[20];

	int AD7991_results[4];
	//AD7991_expected[] = {3.6, 0.068, 5, 3.3}, AD7991_err_margin = 0.5;
	uint16_t results[4];
	
	enum status_code AD7991_code = AD7991_read_all_mV(results, AD7991_BATBRD);

	AD7991_results[0] = ((int)results[0]-985)*2;	//L2_SNS mA
	AD7991_results[1] = ((int)results[1]-985)*2;	//L1_SNS mA
	AD7991_results[2] = ((int)results[2]-50)*2717/1000;	//L_REF mV
	AD7991_results[3] = ((int)results[3]-130)*5580/1000;	//PANELREF mV
	
	get_status(AD7991_code,test_str);
	print("STATUS: \t %s\n",test_str);

	for (int i = 0; i < 4; i++){
		char suffix = i > 1 ? 'V' : 'A';		
		switch (i) {
			case 0 :
			strcpy(test_str,"L2_SNS");
			break;
			case 1 :
			strcpy(test_str,"L1_SNS");
			break;
			case 2 :
			strcpy(test_str,"L_REF ");
			break;
			case 3:
			strcpy(test_str, "PANELREF");
			break;
		}

		print("%s: \t %d mV \t %d m%c\n",test_str, results[i], AD7991_results[i], suffix);
		//if (AD7991_results[i] > AD7991_expected[i]){
		//if((AD7991_results[i] - AD7991_expected[i]) >= AD7991_err_margin) {
		//print("Error in test AD7991 number %d \n",i);
		//pass = 0;
		//}
		//} else {
		//if((AD7991_results[i] - AD7991_expected[i]) >= AD7991_err_margin) {
		//print("Error in test AD7991 number %d \n",i);
		//pass = 0;
		//}
		//}
	}

	//if (pass) {
	//print("AD7991: All tests passed!\n");
	//}
}


static void AD7991_CTRL_test(bool regulatorsOn){

	//pass flag
	char test_str[20];	

	float AD7991_results[4];	
	uint16_t results[4];
	
	if (regulatorsOn) {
		print("==============AD7991_CTRLBRD Test | regulators ON==============\n");
		set_regulator_power(true);
		delay_ms(1000);
	} else {
		print("==============AD7991_CTRLBRD Test | regulators OFF==============\n");
		set_regulator_power(false);
	}

	enum status_code AD7991_code = AD7991_read_all_mV(results, AD7991_CTRLBRD);
	
	AD7991_results[0] = ((float) results[0])*2.01;		//3V6Rf
	AD7991_results[1] = ((float) results[1]);			//3V6SNS
	AD7991_results[2]= ((float)  results[2])*3.381;		//5VREF
	AD7991_results[3] = ((float) results[3])*2.01;		//3V3REF

	set_regulator_power(false);
	
	get_status(AD7991_code,test_str);
	print("STATUS: \t %s\n",test_str);

	//check with expected values
	for (int i = 0; i < 4; i++){
		char suffix = 'V';
		switch (i) {
			case 0 :
			strcpy(test_str,"3V6_REF");
			break;
			case 1 :
			strcpy(test_str,"3V6_SNS");
			suffix = 'A';
			break;
			case 2 :
			strcpy(test_str,"5VREF ");
			break;
			case 3:
			strcpy(test_str, "3V3REF");
			break;
		}

		print("%s: \t %d m%c\n",test_str,(uint16_t)(AD7991_results[i]), suffix);
		//if (AD7991_results[i] > AD7991_expected[i]){
		//if((AD7991_results[i] - AD7991_expected[i]) >= AD7991_err_margin) {
		//print("Error in test AD7991 number %d \n",i);
		//pass = 0;
		//}
		//} else {
		//if((AD7991_results[i] - AD7991_expected[i]) >= AD7991_err_margin) {
		//print("Error in test AD7991 number %d \n",i);
		//pass = 0;
		//}
		//}
	}

	//if (pass) {
	//print("AD7991: All tests passed!\n");
	//}
}

static void readBatBoard(void){
	print("==============BATBRD Test==============\n");
	struct adc_module bat_instance;		
	uint16_t bat_ref_voltage_readings[10];
	float bat_voltage_readings[10];
	char test_str[20];
	char error_str[20];	
	enum adc_positive_input bat_adc_pins[10] = {
		P_AI_LFB1OSNS,
		P_AI_LFB1SNS,
		P_AI_LFB2OSNS,
		P_AI_LFB2SNS,
		P_AI_LF2REF,
		P_AI_LF1REF,		
		P_AI_LF4REF,
		P_AI_LF3REF,
		P_AI_L1_REF,
		P_AI_L2_REF,
	};
	
	for (int i=0; i<10; i++){		
		configure_adc(&bat_instance,bat_adc_pins[i], true);		
		adc_enable(&bat_instance);
		enum status_code sc = read_adc_mV(bat_instance, &bat_ref_voltage_readings[i]);
		get_status(sc,error_str);		
		char suffix = i<4 ? 'A' : 'V';
		switch (i) {
			case 0:
			strcpy(test_str,"LFB1OSNS");
			bat_voltage_readings[i] = bat_ref_voltage_readings[i]*71.43;
			break;
			case 1:
			strcpy(test_str,"LFB1SNS");
			bat_voltage_readings[i] = (bat_ref_voltage_readings[i]-980) * 50;
			break;
			case 2:
			strcpy(test_str,"LFB2OSNS");
			bat_voltage_readings[i] = bat_ref_voltage_readings[i]*71.43;
			break;
			case 3:
			strcpy(test_str,"LFB2SNS");
			bat_voltage_readings[i] = (bat_ref_voltage_readings[i]-979) * 50;
			break;
			case 4:
			strcpy(test_str,"LF2REF");
			bat_voltage_readings[i] = bat_ref_voltage_readings[i] * 1.95;			
			break;		
			case 5:
			strcpy(test_str,"LF1REF");
			bat_voltage_readings[i] = bat_ref_voltage_readings[i] * 3.87 - bat_voltage_readings[i-1];
			break;
			case 6:
			strcpy(test_str,"LF4REF");
			bat_voltage_readings[i] = bat_ref_voltage_readings[i] * 1.95;
			break;			
			case 7:
			strcpy(test_str,"LF3REF");
			bat_voltage_readings[i] = bat_ref_voltage_readings[i] * 3.87 - bat_voltage_readings[i-1];
			break;
			case 8:
			strcpy(test_str,"L1_REF");
			bat_voltage_readings[i] = bat_ref_voltage_readings[i] * 2.5;
			break;
			case 9:
			strcpy(test_str,"L2_REF");
			bat_voltage_readings[i] = bat_ref_voltage_readings[i] * 2.5;
			break;
		}
		print(" %s \t %s \t %4d mV\t %4d m%c\n",test_str,error_str,(uint16_t)(bat_ref_voltage_readings[i]), (int)(bat_voltage_readings[i]), suffix);
	}
}

static void readLEDCurrent(bool printFloats){
	print("==============LED Test==============\n");
	struct adc_module bat_instance;
	uint16_t voltage_readings[4];
	float currentReadings[4];
	char test_str[20];
	char error_str[20];
	enum adc_positive_input bat_adc_pins[4] = {
		P_AI_LED1SNS,
		P_AI_LED2SNS,
		P_AI_LED3SNS,
		P_AI_LED4SNS,
	};
	
	for (int i=0; i<4; i++){		
		configure_adc(&bat_instance,bat_adc_pins[i], true);
		uint8_t rs;
		LTC1380_channel_select(0x4a, i, &rs);
		adc_enable(&bat_instance);
		enum status_code sc = read_adc_mV(bat_instance, &voltage_readings[i]);
		get_status(sc,error_str);
		
		switch (i) {
			case 0:
			strcpy(test_str,"LED1SNS");			
			break;
			case 1:
			strcpy(test_str,"LED2SNS");			
			break;
			case 2:
			strcpy(test_str,"LED3SNS");			
			break;
			case 3:
			strcpy(test_str,"LED4SNS");			
			break;			
		}
		currentReadings[i] = voltage_readings[i]*1000/30;
		if (printFloats) {
			print(" %s \t %s \t %d mV\t %.01f mA\n",test_str,error_str,(voltage_readings[i]), (currentReadings[i]));	
		} else {
			print(" %s \t %s \t %d mV\t %d mA\n",test_str,error_str,(voltage_readings[i]), (uint16_t)(currentReadings[i]));	
		}
	}
}

static void LION_DISG_test(void) {
	print("==============DISCHARGE Test==============\n");

	print("L1 ON, L2 OFF\n");
	set_output(false, P_L1_DISG);
	set_output(true, P_L2_DISG);
	delay_ms(500);
	TCA9535_test(true, false);
	
	print("\nL1 OFF, L2 ON\n");
	set_output(false, P_L2_DISG);
	set_output(true, P_L1_DISG);	
	delay_ms(500);
	TCA9535_test(true, false);
}

static void set_chg_states(bool lion1, bool lion2, bool lifepo_b1, bool lifepo_b2) {	
	set_output(lion1, P_L1_RUN_CHG);	
	set_output(lion2, P_L2_RUN_CHG);
	set_output(lifepo_b1, P_LF_B1_RUNCHG);
	set_output(lifepo_b2, P_LF_B2_RUNCHG);
}

static void CHG_test(void) {
	print("==============CHARGE Test==============\n");

	print("CHARGE L1");
	set_chg_states(true, false, false, false);
	delay_ms(1000);
	TCA9535_test(false, true);
	
	print("\nCHARGE L2");
	set_chg_states(false, true, false, false);
	delay_ms(1000);
	TCA9535_test(false, true);
	
	print("\nCHARGE LFB1");
	set_chg_states(false, false, true, false);
	delay_ms(1000);
	TCA9535_test(false, true);
	
	print("\nCHARGE LFB2");
	set_chg_states(false, false, false, true);
	delay_ms(1000);
	TCA9535_test(false, true);
	
	print("\nCHARGE ALL");
	set_chg_states(true, true, true, true);
	delay_ms(1000);
	TCA9535_test(false, true);
	
	set_chg_states(false, false, false, false);
}

void lf_sns_test(void) {
	set_chg_states(true, true, true, true);
	print("LFB1OSNS, LFB1SNS, LFB2OSNS, LFB2SNS, LF2REF, LF1REF, LF4REF, LF3REF, L1_REF, L2_REF, L2_SNS, L1_SNS\n\r");
	while (true) {
	struct adc_module bat_instance;
	uint16_t bat_ref_voltage_readings[10];
	float bat_voltage_readings[10];	
	enum adc_positive_input bat_adc_pins[10] = {
		P_AI_LFB1OSNS,
		P_AI_LFB1SNS,
		P_AI_LFB2OSNS,
		P_AI_LFB2SNS,
		P_AI_LF2REF,
		P_AI_LF1REF,
		P_AI_LF4REF,
		P_AI_LF3REF,
		P_AI_L1_REF,
		P_AI_L2_REF,
	};
	
	for (int i=0; i<10; i++){
		configure_adc(&bat_instance,bat_adc_pins[i], true);
		uint8_t rs;
		LTC1380_channel_select(0x4a, i, &rs);
		adc_enable(&bat_instance);
		enum status_code sc = read_adc_mV(bat_instance, &bat_ref_voltage_readings[i]);				
		switch (i) {
			case 0:			
			bat_voltage_readings[i] = bat_ref_voltage_readings[i];//*71.43;
			break;
			case 1:			
			bat_voltage_readings[i] = (bat_ref_voltage_readings[i]);//-980) * 50;
			break;
			case 2:			
			bat_voltage_readings[i] = bat_ref_voltage_readings[i];//*71.43;
			break;
			case 3:			
			bat_voltage_readings[i] = (bat_ref_voltage_readings[i]);//-979) * 50;
			break;
			case 4:			
			bat_voltage_readings[i] = bat_ref_voltage_readings[i] * 1.95;
			break;
			case 5:			
			bat_voltage_readings[i] = bat_ref_voltage_readings[i] * 3.87 - bat_voltage_readings[i-1];
			break;
			case 6:			
			bat_voltage_readings[i] = bat_ref_voltage_readings[i] * 1.95;
			break;
			case 7:			
			bat_voltage_readings[i] = bat_ref_voltage_readings[i] * 3.87 - bat_voltage_readings[i-1];
			break;
			case 8:			
			bat_voltage_readings[i] = bat_ref_voltage_readings[i] * 2.5;
			break;
			case 9:			
			bat_voltage_readings[i] = bat_ref_voltage_readings[i] * 2.5;
			break;
		}
		print("%d,",(int)(bat_voltage_readings[i]));
	}	
	
	
	//char test_str[20];

	int AD7991_results[4];
	//AD7991_expected[] = {3.6, 0.068, 5, 3.3}, AD7991_err_margin = 0.5;
	uint16_t results[4];
	
	enum status_code AD7991_code = AD7991_read_all_mV(results, AD7991_BATBRD);

	AD7991_results[0] = ((int)results[0]-985)*2;	//L2_SNS mA
	AD7991_results[1] = ((int)results[1]-985)*2;	//L1_SNS mA
	AD7991_results[2] = ((int)results[2]-50)*2717/1000;	//L_REF mV
	AD7991_results[3] = ((int)results[3]-130)*5580/1000;	//PANELREF mV
			

	for (int i = 0; i < 2; i++){
		print("%d,",results[i]);
	}
	print("\n\r");
	delay_ms(10000);
	}
}

void system_test(bool printFloats){		
	#ifndef XPLAINED
	print("=======================================\n");
	print("=               SYSTEM Test           =\n");
	print("=======================================\n");		
	readBatBoard();
	AD7991_BAT_test();
	
	AD7991_CTRL_test(false);
	AD7991_CTRL_test(true);
	
	readLEDCurrent(printFloats);
	
	AD590_test();
			
	MLX90614_test(printFloats);
	
	MPU9250_test(false, printFloats);	
	HMC5883L_test(printFloats);	
	
	TEMD6200_test();
	
	TCA9535_test(true, true);
	
	LION_DISG_test();
	
	CHG_test();
	#endif
}
