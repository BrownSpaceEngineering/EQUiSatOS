/*
 * system_test.c
 *
 * Created: 10/22/2017 1:15:00 PM
 *  Author: Jarod Boone
 */
#include "system_test.h"

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
static void get_panel(int panel_addr, char* buffer){

	switch (panel_addr){

		case MLX90614_FLASHPANEL_V6_2_1:
		strcpy(buffer,"IR_FLASH");
		break;

		case MLX90614_TOPPANEL_V5_1:
		strcpy(buffer,"IR_TOP1");
		break;

		case MLX90614_RBFPANEL_V1:
		strcpy(buffer,"IR_RBF");
		break;

		case MLX90614_ACCESSPANEL_V4_7:
		strcpy(buffer,"IR_ACCESS");
		break;

		case MLX90614_SIDEPANEL_V5_2:
		strcpy(buffer,"IR_SIDE1");
		break;

		case MLX90614_SIDEPANEL_V5_5:
		strcpy(buffer,"IR_SIDE2");
		break;

		case MLX90614_ACCESSPANEL_V4_6:
		strcpy(buffer,"F_IR_ACCESS");
		break;

		case MLX90614_TOPPANEL_V4_1:
		strcpy(buffer,"F_IR_TOP1");
		break;

		default:
		strcpy(buffer,"Unknown IR Address");
	}
}

// test all addresses in the array (addr) and fill results in (results) specify number of address input with (num)
static void MLX90614_test(uint8_t * addr, int num){

	 //read the value from the sensor specified by (addr) and put value in rs
	 uint16_t buf;
	 char buffer[20];
	 enum status_code sc;
	 for (int i = 0; i < num; i++){
		sc = MLX90614_read_all_obj(addr[i],&buf);

		get_panel(addr[i],buffer); // get panel address string
		print("%s: \t ",buffer);
		get_error(sc,buffer); // get error code string
		print("%s \t %d\t",buffer,(uint16_t) dataToTemp(buf));

		sc = MLX90614_read_amb(addr[i],&buf);
		get_error(sc,buffer);
		print("%s \t %d\n",buffer,(uint16_t)dataToTemp(buf));
	 }
}


static float AD590_test(int channel, char * buffer){
	struct adc_module temp_instance; //generate object
	// TEST BOARD ADC MUX
	//setup_pin(true, 51);
	//set_output(true, 51);
	//setup_pin(true, 49);
	//set_output(true, 49);
	//setup_pin(true, 50);
	//set_output(false, 50);
		//
	////Set up object to adc pin
	//configure_adc(&temp_instance,ADC_POSITIVE_INPUT_PIN8); //IS THIS THE RIGHT PIN???? -> probably not lmao
		//
	////try reading
	//return read_adc(temp_instance);

	configure_adc(&temp_instance,P_AI_TEMP_OUT);
	uint8_t rs;
	LTC1380_channel_select(TEMP_MULTIPLEXER_I2C, channel, &rs);

	uint16_t temp;
	enum status_code sc = read_adc(temp_instance,&temp);
	get_error(sc,buffer);

	// I do not know if this was done yet :/
	// temperature conversion from voltage -> current -> degrees celcius
	float current = ((float) convertToVoltage(temp))/2197 -0.0001001; //converts from V to A
	return (current)*1000000-273;// T = 454*V in C
}

//IMU test if rebias = 0 then don't compute the bias again
// input length 3 array (sensors) each array entry corresponds to a sensor, 0 means don't read, else read
// sensors[0] -> acc, sensors[1] -> gyro, sensors[3] -> mag  (length of toFill should correspond to sensor reads)
static void MPU9250_test(uint16_t toFill[],int* sensors, int rebias){
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
		toFill[0] = reader[0];
		toFill[1] = reader[1];
		toFill[2] = reader[2];

		// status code ?
		get_error(code,buffer);
		print("acc \t %s \t x: %d \t y: %d \t z: %d\n",buffer,toFill[0],toFill[1],toFill[2]);
		//print_error(code);
	}

	if (sensors[1]){
		code = MPU9250_read_gyro(reader);
		toFill[3] = reader[0];
		toFill[4] = reader[1];
		toFill[5] = reader[2];

		// status code?
		get_error(code,buffer);
		print("gyro \t %s \t x: %d \t y: %d \t z: %d\n",buffer,toFill[3],toFill[4],toFill[5]);
		//print_error(code);
	}

	if (sensors[2]){
		code = MPU9250_read_mag(reader);
		toFill[6] = reader[0];
		toFill[7] = reader[1];
		toFill[8] = reader[2];

		// status code?
		print("MPU9250 read magnetometer");
		//print_error(code);
	}


}

//Magnetometer test
static float HMC5883L_test(void){

	uint8_t buffer[6];
	int16_t xyz[3];

	HMC5883L_init();
	HMC5883L_read(buffer); // pretty sure this doesn't do anything ~l m a o~
	HMC5883L_getXYZ(buffer, xyz);
	return HMC5883L_computeCompassDir(xyz[0],xyz[1],xyz[2]);

}

//GPIO test
static enum status_code TCA9535_test(uint16_t* rs){
	return TCA9535_init(rs);
}

// Photodiode test
static float TEMD6200_test(int channel){
	struct adc_module pd_instance;

	configure_adc(&pd_instance,P_AI_PD_OUT);
	uint8_t rs;
	LTC1380_channel_select(0x4a, channel, &rs);
	//pdBuffer[i] =(readVoltagemV(pd_instance));//-6.5105)/0.3708; // I = exp((V-6.5105/0.3708)) in uA

	uint16_t read;

	adc_enable(&pd_instance);
	uint16_t buf;
	read_adc(pd_instance, &read);

	return convertToVoltage(read);
}



static enum status_code AD7991_control_test_all(float *results_f){

	uint16_t results[4];

	AD7991_init();
	enum status_code read = AD7991_read_all(results, AD7991_ADDR_1);

	results_f[0] = ((float) results[0])/4096*3.3*2.01;//3V6Rf
	results_f[1] = ((float) results[1])/4096*3.3;//3V6SNS
	results_f[2]= ((float)  results[2])/4096*3.3*3.381;//5VREF
	results_f[3] = ((float) results[3])/4096*3.3*2.01;//3V3REF

	set_output(false, P_RAD_PWR_RUN);
	set_output(false, P_5V_EN);


}

//
void system_test_init(void){
	setup_pin(true, P_RAD_PWR_RUN); //3.6V regulator
	setup_pin(true, P_5V_EN); // 5V regulator
}

void system_test(void){

	system_test_init();

	MLX90614_init();
	configure_i2c_master(SERCOM4); //init I2C
	LTC1380_init(); //init multiplexer

	print("AD7991 test========================================\n");
	//pass flag
	int pass = 1;
	char test_str[20];

	float AD7991_results[4], AD7991_expected[] = {3.6, 0.068, 5, 3.3}, AD7991_err_margin = 0.5;

	enum status_code AD7991_code = AD7991_control_test_all(AD7991_results);
	get_error(AD7991_code,test_str);
	print("STATUS: \t %s\n",test_str);

	//check with expected values
	for (int i = 0; i < 4; i++){

		switch (i) {
			case 0 :
			strcpy(test_str,"3V6_REF");
			break;
			case 1 :
			strcpy(test_str,"3V6_SNS");
			break;
			case 2 :
			strcpy(test_str,"5VREF");
			break;
			case 3:
			strcpy(test_str, "3V3REF");
			break;
		}

		print("%s: \t %d mV\n",test_str,(1000 * AD7991_results[i]));
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

	const uint16_t expected_temp = 20;//Celsius

	float temps[8];
	char error_str[20];

	//	set_output(true, P_RAD_PWR_RUN);
	set_output(true, P_5V_EN);
	//ADC out of comission at the moment
	print("AD590 test========================================\n");
	for (int i = 0; i < 8; i++){
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

		temps[i] = AD590_test(i,error_str);
		print(" %s \t %s \t %d degrees Celsius\n",test_str,error_str,temps[i]);
	}

	// compare_results((void *) temps,(void *) expected,4, 5, "AD590");
	print("MLX90614 test========================================\n");
	print("Panel Name \t Obj Status \t Obj \t Amb Status \t Amb \n");
	//#define MLX90614_FLASHPANEL_V6_2_1	0x6C
	//uint8_t addr[] = {MLX90614_SIDEPANEL_V5_2,MLX90614_SIDEPANEL_V5_5,
		//MLX90614_ACCESSPANEL_V4_7,MLX90614_FLASHPANEL_V6_2_1,
		//MLX90614_RBFPANEL_V1,MLX90614_TOPPANEL_V5_1};

	uint8_t addr[] = {MLX90614_TOPPANEL_V4_1,MLX90614_ACCESSPANEL_V4_6};

	// arg 2 should be the size of addr
	MLX90614_test(addr,2);


	print("MPU9250 test========================================\n");
	print("Accelerometer readings in g, gyroscope readings in degrees per second\n");
	// IMU
	// testmap 0 - acc x : 1 - acc y : 2 - acc z : 3 - gyr x : 4 - gyr y : 5 - gyr z
	uint16_t MPU9250_results[6], MPU9250_err_margin = 20;
	int sensors[] = {1, 1, 0};
	MPU9250_test(MPU9250_results,sensors,0);
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


	//WILL THESE WORK????
	print("HMC5883L test========================================\n");
	float test4 = HMC5883L_test();
	print("HMC test: %d\n",test4);

	//ADC out of commission
	print("TEMD6200 test========================================\n");
	float pd_tests[6];
	for (int i = 0; i < 6; i++){

		switch (i) {
			case 0:
			strcpy(test_str,"PD_FLASH");
			break;
			case 1:
			strcpy(test_str,"PD_SIDE1");
			break;
			case 2:
			strcpy(test_str,"PD_SIDE2");
			break;
			case 3:
			strcpy(test_str,"PD_ACCESS");
			break;
			case 4:
			strcpy(test_str,"PD_TOP1");
			break;
			case 5:
			strcpy(test_str,"PD_TOP2");
			break;
		}

		pd_tests[i] = TEMD6200_test(i);
		print("TEMD6200 test %s: %3.5f \n",test_str, pd_tests[i]);
	}
	//uint16_t test5 = TEMD6200_test(0,5);


	print("TCA9535 test========================================\n");
	uint16_t rs;
	enum status_code sc = TCA9535_test(&rs);
	print("TCA return status: ");
	//print_error(sc);
	print("TCA test: %d\n",rs);
	return;
}
