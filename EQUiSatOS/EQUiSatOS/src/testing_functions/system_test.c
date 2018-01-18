#include "system_test.h"

void sensor_read_tests(void) {
	uint16_t six_buf[6];
	uint8_t six_buf_8t[6];
	uint8_t three_buf[3];
	uint8_t four_buf[4];
	uint8_t two_buf[2];
	delay_ms(100);
	print("\n\n\n\n##### NEW RUN #####\n");
	
	print("\n# IMU #\n");
	read_accel_batch(three_buf);
	print("accel: %d %d %d\n", (int16_t)((int8_t) three_buf[0]) << 8, (int16_t)((int8_t) three_buf[1]) << 8, (int16_t)((int8_t) three_buf[2]) << 8);
	read_gyro_batch(three_buf);
	print("gyro: %d %d %d\n", (int16_t)((int8_t) three_buf[0]) << 8, (int16_t)((int8_t) three_buf[1]) << 8, (int16_t)((int8_t) three_buf[2]) << 8);
	read_magnetometer_batch(three_buf);
	print("mag: %d %d %d\n", (int16_t)((int8_t) three_buf[0]) << 8, (int16_t)((int8_t) three_buf[1]) << 8, (int16_t)((int8_t) three_buf[2]) << 8);

	print("\n# IR #\n");
	read_ir_ambient_temps_batch(six_buf);
	print("ir ambs: %d %d %d %d %d %d\n", (uint16_t)dataToTemp(six_buf[0] << 8), (uint16_t)dataToTemp(six_buf[1] << 8), (uint16_t)dataToTemp(six_buf[2] << 8), (uint16_t)dataToTemp(six_buf[3] << 8), (uint16_t)dataToTemp(six_buf[4] << 8), (uint16_t)dataToTemp(six_buf[5] << 8));
	read_ir_object_temps_batch(six_buf);
	print("ir objs: %d %d %d %d %d %d\n", (uint16_t)dataToTemp(six_buf[0] << 8), (uint16_t)dataToTemp(six_buf[1] << 8), (uint16_t)dataToTemp(six_buf[2] << 8), (uint16_t)dataToTemp(six_buf[3] << 8), (uint16_t)dataToTemp(six_buf[4] << 8), (uint16_t)dataToTemp(six_buf[5] << 8));

	print("\n# PDIODE #\n");
	read_pdiode_batch(six_buf_8t);
	for (int i = 0; i < 6; i++){
		print("pdiode %d: %d\n",i, (uint16_t)(six_buf_8t[i])<<8);
	}

	print("\n\n# LiON VOLTS #\n");
	read_lion_volts_batch(two_buf);
	print("lion volts: %d %d\n", two_buf[0], two_buf[1]);

	print("# LiON CURRENT #\n");
	read_lion_current_batch(two_buf);
	print("lion current: %d %d\n", two_buf[0], two_buf[1]);

	print("# LiON TEMPS #\n");
	read_lion_temps_batch(two_buf);
	print("lion temps: %d %d\n", two_buf[0], two_buf[1]);


	print("\n# LiFePO VOLTS #\n");
	read_lifepo_volts_batch(four_buf);
	print("lifepo volts: %d %d %d %d\n", four_buf[0], four_buf[1], four_buf[2], four_buf[3]);

	print("# LiFePO CURRENT #\n");
	read_lifepo_current_batch(four_buf);
	print("lifepo current: %d %d %d %d\n", four_buf[0], four_buf[1], four_buf[2], four_buf[3]);
}

// Function that takes in a status code and prints out the matching status
static void get_error(enum status_code code, char * buffer){
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

		case IR_FLASH:
		strcpy(buffer,"IR_FLASH");
		break;

		case IR_TOP1:
		strcpy(buffer,"IR_TOP1");
		break;

		case IR_RBF:
		strcpy(buffer,"IR_RBF");
		break;

		case IR_ACCESS:
		strcpy(buffer,"IR_ACCESS");
		break;

		case IR_SIDE1:
		strcpy(buffer,"IR_SIDE1");
		break;

		case IR_SIDE2:
		strcpy(buffer,"IR_SIDE2");
		break;
		
		default:
		strcpy(buffer,"Unknown IR Address");
	}
}

// test all addresses in the array (addr) and fill results in (results) specify number of address input with (num)
static void MLX90614_test(void){
	setup_pin(true, P_IR_PWR_CMD);
	set_output(true, P_IR_PWR_CMD);
	print("==============MLX90614 Test==============\n");
	print("Panel Name \t Obj Status \t Obj \t Amb Status \t Amb \n");

	//read the value from the sensor specified by (addr) and put value in rs
	uint16_t buf;
	char buffer[40];
	enum status_code sc;
	uint8_t addr[] = {IR_FLASH, IR_SIDE1, IR_SIDE2, IR_RBF, IR_ACCESS, IR_TOP1};

	for (int i = 0; i < 6; i++){
		sc = MLX90614_read_all_obj(addr[i],&buf);

		get_ir_panel(addr[i],buffer); // get panel address string
		print("%s: \t ",buffer);
		get_error(sc,buffer); // get error code string
		print("%s \t %d\t",buffer,(uint16_t) dataToTemp(buf));

		sc = MLX90614_read_amb(addr[i],&buf);
		get_error(sc,buffer);
		print("%s \t %d\n",buffer,(uint16_t)dataToTemp(buf));
	}
}


static void AD590_test(void){
	char test_str[20];
	const uint16_t expected_temp = 20;//Celsius
	char error_str[20];	
	struct adc_module temp_instance; //generate object	
	
	set_output(true, P_5V_EN);
	
	print("==============AD590 Test==============\n");
	for (int i = 0; i < 8; i++){
		configure_adc(&temp_instance,P_AI_TEMP_OUT);
		uint8_t rs;
		LTC1380_channel_select(TEMP_MULTIPLEXER_I2C, i, &rs);		
		
		delay_ms(1);

		uint16_t temp_mV;
		enum status_code sc = read_adc_mV(temp_instance,&temp_mV);
		
		// temperature conversion from voltage -> current -> degrees celsius		
		float current = ((float)temp_mV)/1000/2197. -0.000153704; //converts from V to A
		float tempInC = (current)*1000000-273;// T = 454*V in C
		
		get_error(sc,error_str);	
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
static void MPU9250_test(bool rebias){
	print("==============MPU9250 Test==============\n");
	print("Accelerometer readings in g, gyroscope readings in degrees per second\n");
	// IMU
	// testmap 0 - acc x : 1 - acc y : 2 - acc z : 3 - gyr x : 4 - gyr y : 5 - gyr z
	uint16_t MPU9250_results[6], MPU9250_err_margin = 20;
	int sensors[] = {1, 1, 0};
		
	//initalize imu.... probably
	MPU9250_init();

	// re compute bias on call
	if (rebias) {
		uint16_t gyroBias[3] = {0, 0, 0}, accelBias[3] = {0, 0, 0};
		MPU9250_computeBias(gyroBias,accelBias);
	}

	uint16_t reader[3];
	char buffer[20];
	enum status_code code;

	if (sensors[0]){
		code = MPU9250_read_acc(reader);
		MPU9250_results[0] = reader[0];
		MPU9250_results[1] = reader[1];
		MPU9250_results[2] = reader[2];

		// status code ?
		get_error(code,buffer);
		print("acc \t %s \t x: %d \t y: %d \t z: %d\n",buffer,(int16_t) MPU9250_results[0],(int16_t) MPU9250_results[1],(int16_t) MPU9250_results[2]);
		//print_error(code);
	}

	if (sensors[1]){
		code = MPU9250_read_gyro(reader);
		MPU9250_results[3] = reader[0];
		MPU9250_results[4] = reader[1];
		MPU9250_results[5] = reader[2];

		// status code?
		get_error(code,buffer);
		print("gyro \t %s \t x: %d \t y: %d \t z: %d\n",buffer,(int16_t) MPU9250_results[3],(int16_t) MPU9250_results[4],(int16_t) MPU9250_results[5]);
		//print_error(code);
	}

	if (sensors[2]){
		code = MPU9250_read_mag(reader);
		MPU9250_results[6] = reader[0];
		MPU9250_results[7] = reader[1];
		MPU9250_results[8] = reader[2];

		// status code?
		print("MPU9250 read magnetometer");
		//print_error(code);
	}
	
	//uint16_t MPU9250_expected[] = {0, 0, 0, 0, 0, 0};
	//for (int i = 0; i < 6; i++){
	//
	//switch (i) {
	//case 0:
	//strcpy(test_str,"acc x");
	//break;
	//case 1:
	//strcpy(test_str,"acc y");
	//break;
	//case 2:
	//strcpy(test_str,"acc z");
	//break;
	//case 3:
	//strcpy(test_str,"gyro x");
	//break;
	//case 4:
	//strcpy(test_str,"acc y");
	//break;
	//case 5:
	//strcpy(test_str,"acc z");
	//break;
	//}
	//print("MPU Reading %s: %d\n",test_str,MPU9250_results[i]);
	//
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
	//}
	//}


}

//Magnetometer test
static void HMC5883L_test(void){
	print("==============HMC5883L Test==============\n");
	uint8_t buffer[6];
	int16_t xyz[3];

	HMC5883L_init();
	HMC5883L_read(buffer);
	HMC5883L_getXYZ(buffer, xyz);
	print("x: %d \t y: %d \t z: %d \n", xyz[0],xyz[1],xyz[2]);

}

//GPIO test
static enum status_code TCA9535_test(void){
	print("==============TCA9535 Test==============\n");
	uint16_t res;
	enum status_code sc = TCA9535_init(&res);
	print("TCA return status: ");
	print_error(sc);
	print("L2_ST: \t\t %d\n", (res>>8)&0x1);
	print("L1_ST: \t\t %d\n", (res>>9)&0x1);
	print("SPF_ST: \t %d\n", (res>>10)&0x1);
	print("L1_CHGN: \t %d\n", (res>>12)&0x1);
	print("L1_FAULTN: \t %d\n", (res>>13)&0x1);
	print("L2_CHGN: \t %d\n", (res>>14)&0x1);
	print("L2_FAULTN: \t %d\n", (res>>15)&0x1);
	print("LF_B1_CHGN: \t %d\n", (res>>7)&0x1);
	print("LF_B1_FAULTN: \t %d\n", (res>>6)&0x1);
	print("LF_B2_CHGN: \t %d\n", (res>>4)&0x1);
	print("LF_B2_FAULTN: \t %d\n", (res>>5)&0x1);	
	//print("TCA test: %d\n",rs);
}

// Photodiode test
static float TEMD6200_test(void){
	print("==============TEMD6200 Test==============\n");
	char test_str[20];
	struct adc_module pd_instance;
	for (int i = 0; i < 6; i++){		
		uint16_t pd_mV;
		configure_adc(&pd_instance,P_AI_PD_OUT);
		uint8_t rs;		
		LTC1380_channel_select(0x4a, i, &rs);
		adc_enable(&pd_instance);
		read_adc_mV(pd_instance, &pd_mV);
		switch (i) {
			case 0:
			strcpy(test_str,"PD_ACCESS");
			break;
			case 1:
			strcpy(test_str,"PD_SIDE1");
			break;
			case 2:
			strcpy(test_str,"PD_SIDE2");
			break;
			case 3:
			strcpy(test_str,"PD_FLASH");
			break;
			case 4:
			strcpy(test_str,"PD_TOP1");
			break;
			case 5:
			strcpy(test_str,"PD_RBF\t");
			break;
		}
		print("%s \t %d \n",test_str, pd_mV);
	}
	
	//pdBuffer[i] =(readVoltagemV(pd_instance));//-6.5105)/0.3708; // I = exp((V-6.5105/0.3708)) in uA	
}

static void AD7991_BAT_test(void){

	print("==============AD7991_BATBRD Test==============\n");
	//pass flag
	int pass = 1;
	char test_str[20];

	int AD7991_results[4];
	//AD7991_expected[] = {3.6, 0.068, 5, 3.3}, AD7991_err_margin = 0.5;
	uint16_t results[4];
	
	enum status_code AD7991_code = AD7991_read_all_mV(results, AD7991_BATBRD);

	AD7991_results[0] = ((int)results[0]-1022)*2;	//L2_SNS mA
	AD7991_results[1] = ((int)results[1]-985)*2;	//L1_SNS mA
	AD7991_results[2] = ((int)results[2]-50)*2717/1000;	//L_REF mV
	AD7991_results[3] = ((int)results[3]-130)*5580/1000;	//PANELREF mV
	
	get_error(AD7991_code,test_str);
	print("STATUS: \t %s\n",test_str);

	for (int i = 0; i < 4; i++){
		char suffix;
		if (i > 1) {
			suffix = 'V';
		} else {
			suffix = 'A';
		}
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

		print("%s: \t %d \t %d m%c\n",test_str, results[i], AD7991_results[i], suffix);
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
	int pass = 1;
	char test_str[20];	

	float AD7991_results[4], AD7991_expected[] = {3.6, 0.068, 5, 3.3}, AD7991_err_margin = 0.5;	
	uint16_t results[4];
	
	if (regulatorsOn) {
		print("==============AD7991_CTRLBRD Test | regulators ON==============\n");
		set3V6Power(true);
		setRadioPower(true);
		set_output(true, P_5V_EN);
		delay_ms(1000);
	} else {
		print("==============AD7991_CTRLBRD Test | regulators OFF==============\n");
		setRadioPower(false);
		set3V6Power(false);		
		set_output(false, P_5V_EN);
	}

	enum status_code AD7991_code = AD7991_read_all_mV(results, AD7991_CTRLBRD);
	
	AD7991_results[0] = ((float) results[0])*2.01;		//3V6Rf
	AD7991_results[1] = ((float) results[1]);		//3V6SNS
	AD7991_results[2]= ((float)  results[2])*3.381;		//5VREF
	AD7991_results[3] = ((float) results[3])*2.01;		//3V3REF

	set3V6Power(false);
	setRadioPower(false);
	set_output(false, P_5V_EN);
	
	get_error(AD7991_code,test_str);
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

void readBatBoard(void){
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
		uint16_t buf;
		configure_adc(&bat_instance,bat_adc_pins[i]);
		uint8_t rs;
		LTC1380_channel_select(0x4a, i, &rs);
		adc_enable(&bat_instance);
		enum status_code sc = read_adc_mV(bat_instance, &bat_ref_voltage_readings[i]);
		get_error(sc,error_str);		
		
		switch (i) {
			case 0:
			strcpy(test_str,"P_AI_LFB1OSNS");
			bat_voltage_readings[i] = bat_ref_voltage_readings[i]*71.43;
			break;
			case 1:
			strcpy(test_str,"P_AI_LFB1SNS");
			bat_voltage_readings[i] = bat_ref_voltage_readings[i]-0.980 * 50;
			break;
			case 2:
			strcpy(test_str,"P_AI_LFB2OSNS");
			bat_voltage_readings[i] = bat_ref_voltage_readings[i]*71.43;
			break;
			case 3:
			strcpy(test_str,"P_AI_LFB2SNS");
			bat_voltage_readings[i] = bat_ref_voltage_readings[i]-0.979 * 50;
			break;
			case 4:
			strcpy(test_str,"P_AI_LF2REF");
			bat_voltage_readings[i] = bat_ref_voltage_readings[i] * 1.95;
			break;			
			case 5:
			strcpy(test_str,"P_AI_LF1REF");
			bat_voltage_readings[i] = bat_ref_voltage_readings[i] * 3.87-bat_voltage_readings[i-1];
			break;
			case 6:
			strcpy(test_str,"P_AI_LF4REF");
			bat_voltage_readings[i] = bat_ref_voltage_readings[i] * 1.95;
			break;			
			case 7:
			strcpy(test_str,"P_AI_LF3REF");
			bat_voltage_readings[i] = bat_ref_voltage_readings[i] * 3.87-bat_voltage_readings[i-1];
			break;
			case 8:
			strcpy(test_str,"P_AI_L1_REF");
			bat_voltage_readings[i] = bat_ref_voltage_readings[i] * 2.5;
			break;
			case 9:
			strcpy(test_str,"P_AI_L2_REF");
			bat_voltage_readings[i] = bat_ref_voltage_readings[i] * 2.5;
			break;
		}
		print(" %s \t %s \t %4d mV\t %4d mV\n",test_str,error_str,(uint16_t)(bat_ref_voltage_readings[i]), (uint16_t)(bat_voltage_readings[i]));
	}
	
}

void system_test(void){		
	print("=======================================\n");
	print("=               SYSTEM Test           =\n");
	print("=======================================\n");		
	readBatBoard();
	
	AD7991_CTRL_test(false);
	AD7991_CTRL_test(true);
	
	AD7991_BAT_test();
	
	AD590_test();
			
	MLX90614_test();
	
	MPU9250_test(false);	
	HMC5883L_test();	
	
	TEMD6200_test();
	
	TCA9535_test();
}
