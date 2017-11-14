/*
 * system_test.c
 *
 * Created: 10/22/2017 1:15:00 PM
 *  Author: Jarod Boone
 */ 
#include "system_test.h"

// Function that takes in a status code and prints out the matching status
static void print_error(enum status_code code){
	switch(code){
		
		case STATUS_OK:
		print("Status OK\n");
		break;
		
		case STATUS_ERR_BAD_ADDRESS:
		print("Status error, bad address\n");
		break;
		
		case STATUS_ERR_BAD_DATA:
		print("Status error, bad data\n");
		break;
		
		case STATUS_ERR_BAD_FORMAT:
		print("Status error, bad format\n");
		break;
		
		case STATUS_BUSY:
		print("Status busy\n");
		break;
		
		case STATUS_ERR_PACKET_COLLISION:
		print("Status error, packet collision\n");
		break;
		
		default:
		print("Status unknown error \n");
	}
		
}

// function that take in a panel address (hex) and puts the name of that pannel into buffer
static void get_panel(int panel_addr, char* buffer){

	switch (panel_addr){
		case 0x6C:
		buffer = "MLX90614_FLASHPANEL_V6_2_1";
		
		case 0x6B:
		buffer = "MLX90614_TOPPANEL_V4_2";
		
		case 0x6A:
		buffer = "MLX90614_TOPPANEL_V4_1";
		
		case 0x5C:
		buffer = "MLX90614_ACCESSPANEL_V4_1";
		
		case 0x5D: 
		buffer = "MLX90614_SIDEPANEL_V4_2";
		
		case 0x5F: 
		buffer = "MLX90614_SIDEPANEL_V4_3";
		
		case 0x6D:
		buffer = "MLX90614_SIDEPANEL_V4_4";
		
		default: 
		buffer = "INVALID FLASH PANEL ADDRESS";
	}
	
}

static uint16_t MLX90614_test(uint8_t addr){
	 
	 //configure_i2c_standard(SERCOM4); //SERCOM4 -> I2C serial port
	 
	 //DON'T NEED TO TURN ON FOR TEST BOARD. UNCOMMENT WHEN REAL TESTING
	 //MLX90614_init(); //Turns on the IR sensor and sets up the Port DO THIS IN MAIN()
	 
	 //read the value from the sensor specified by (addr) and put value in rs
	 uint16_t rs;
	 enum status_code sc = MLX90614_read_all_obj(addr,&rs);
	 
	 // what was the status code of the read?
	 print("MLX90614 status: ");
	 print_error(sc); 
	 
	 //If return status is return status is category OK
	 if ((sc & 0x0F) != 0){
		 return rs; 
	 }
	 
	 return 0;  
}


static uint16_t AD590_test(int channel){
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
	LTC1380_channel_select(0x48, channel, &rs);
	
	uint16_t temp;
	read_adc(temp_instance, &temp);
	
	// temperature conversion from voltage -> current -> degrees celcius 
	float current = ((float) convertToVoltage(temp))/2197 -0.0001001; //converts from V to A
	return (current)*1000000-273;// T = 454*V in C
}

//IMU test if rebias = 0 then don't compute the bias again 
// input length 3 array (sensors) each array entry corresponds to a sensor, 0 means don't read, else read
// sensors[0] -> acc, sensors[1] -> gyro, sensors[3] -> mag  (length of toFill should correspond to sensor reads) 
static void MPU9250_test(uint16_t toFill[],int[] sensors, int rebias){
	//initalize imu.... probably
	MPU9250_init();
	
	// re compute bias on call 
	if (rebias) {
		uint16_t gyroBias[3] = {0, 0, 0}, accelBias[3] = {0, 0, 0};
		MPU9250_computeBias(gyroBias,accelBias);
	}
		
	uint16_t reader[3]; 
	enum status_code code; 
	
	if (sensors[0]){
		code = MPU9250_read_acc(reader);
		toFill[0] = reader[0];
		toFill[1] = reader[1];
		toFill[2] = reader[2];
		
		// status code ? 
		print("MPU9250 read accelerometer");
		print_error(code);
	} 
	
	if (sensors[1]){
		code = MPU9250_read_gyro(reader);
		toFill[3] = reader[0];
		toFill[4] = reader[1];
		toFill[5] = reader[2];
		
		// status code? 
		print("MPU9250 read gyroscope");
		print_error(code);
	}
	
	if (sensors[2]){
		code = MPU9250_read_mag(reader);
		toFill[6] = reader[0];
		toFill[7] = reader[1];
		toFill[8] = reader[2];
		
		// status code?
		print("MPU9250 read magnetometer");
		print_error(code);
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
static uint16_t TEMD6200_test(int channel, int num_samples){
	struct adc_module pd_instance; 
	
	configure_adc(&pd_instance,P_AI_PD_OUT);
	uint8_t rs;
	LTC1380_channel_select(0x4a, channel, &rs);
	//pdBuffer[i] =(readVoltagemV(pd_instance));//-6.5105)/0.3708; // I = exp((V-6.5105/0.3708)) in uA
			 
	uint16_t sum =0;
			 
	for (int j=0; j<num_samples; j++){
		adc_enable(&pd_instance);
		uint16_t buf;
		read_adc(pd_instance, &buf);
		sum = sum + buf;
	}
	
	return sum/num_samples;	
}



static void AD7991_control_test_all(float *results_f){
	
	uint16_t results[4]; 
	
	setup_pin(true, P_RAD_PWR_RUN); //3.6V regulator 
	setup_pin(true, P_5V_EN); // 5V regulator 
		
	set_output(true, P_RAD_PWR_RUN);		
	set_output(true, P_5V_EN);
	
	AD7991_init(); 
	AD7991_read_all(results, AD7991_ADDR_1);
	
	results_f[0] = ((float) results[0])/4096*3.3*2.01;//3V6Rf
	results_f[1] = ((float) results[1])/4096*3.3;//3V6SNS
	results_f[2]= ((float)  results[2])/4096*3.3*3.381;//5VREF
	results_f[3] = ((float) results[3])/4096*3.3*2.01;//3V3REF	
	
}

void system_test(void){
	
	print("Initialize system tests...\n"); 
	
	MLX90614_init();
	configure_i2c_master(SERCOM4); //init I2C
	LTC1380_init(); //init multiplexer
	
	print("AD7991 test========================================\n"); 
	//pass flag
	int pass = 1; 
	char * test_str;

	float AD7991_results[4], AD7991_expected[] = {3.6, 0.068, 5, 3.3}, AD7991_err_margin = 0.5; 
	
	AD7991_control_test_all(AD7991_results);
	
	//check with expected values
	for (int i = 0; i < 4; i++){
		if (AD7991_results[i] > AD7991_expected[i]){
			if((AD7991_results[i] - AD7991_expected[i]) >= AD7991_err_margin) {
				print("Error in test AD7991 number %d \n",i);
				pass = 0; 
			}
			} else {
			if((AD7991_results[i] - AD7991_expected[i]) >= AD7991_err_margin) {
				print("Error in test AD7991 number %d \n",i);
				pass = 0;
			}
		}
	}
	
	if (pass) {
		print("AD7991: All tests passed!\n");
	}
	
	const uint16_t expected_temp = 20;//Celsius
	uint16_t temps[8];
	
	
	//ADC out of comission at the moment
	print("AD590 test========================================\n");
	for (int i = 0; i < 8; i++){
		switch (i) {
			case 0:
			test_str = "LED1TEMP";
			break;
			case 1:
			test_str = "LED2TEMP";
			break;
			case 2:
			test_str = "LED3TEMP";
			break;
			case 3:
			test_str = "LED4TEMP";
			break;
			case 4:
			test_str = "L1_TEMP";
			break;
			case 5:
			test_str = "L2_TEMP";
			break;
			case 6: 
			test_str = "LF1_TEMP";
			break; 
			case 7: 
			test_str = "LF3_TEMP";
			break; 
			
		}
				
		temps[i] = AD590_test(i);
		print("AD590 test %s: %d degrees Celsius\n",test_str,temps[i]); 
	}
	
	//compare_results((void *) temps,(void *) expected,4, 5, "AD590");
	print("MLX90614 test========================================\n"); 
	////Flight IR Sensors
	//#define MLX90614_FLASHPANEL_V6_2_1	0x6C
	//#define MLX90614_TOPPANEL_V4_2		0x6B //
	//#define MLX90614_TOPPANEL_V4_1		0x6A
	//#define MLX90614_ACCESSPANEL_V4_1	0x5C
	//#define MLX90614_SIDEPANEL_V4_2		0x5D //
	//#define MLX90614_SIDEPANEL_V4_3		0x5F
	//#define MLX90614_SIDEPANEL_V4_4		0x6D
	float test1 = MLX90614_test(MLX90614_ACCESSPANEL_V4_1);
	get_panel(MLX90614_ACCESSPANEL_V4_1,test_str);
	print("IR test on %s yielded approx %d degrees Celsius\n",test_str,(int) test1);
	float test2 = MLX90614_test(MLX90614_FLASHPANEL_V6_2_1);
	get_panel(MLX90614_FLASHPANEL_V6_2_1,test_str);
	print("IR test on %s yielded approx %d degrees Celsius\n",test_str,(int) test2);
	
	
	
	print("MPU9250 test========================================\n");
	print("Accelerometer readings in g, gyroscope readings in degrees per second");
	// IMU 
	// testmap 0 - acc x : 1 - acc y : 2 - acc z : 3 - gyr x : 4 - gyr y : 5 - gyr z
	uint16_t MPU9250_results[6], MPU9250_err_margin = 20; 
	MPU9250_test(MPU9250_results);
	uint16_t MPU9250_expected[] = {0, 0, 0, 0, 0, 0}; 	
	for (int i = 0; i < 6; i++){
		
		switch (i) {
			case 0:
			test_str = "acc x";
			break;
			case 1:
			test_str = "acc y";
			break;
			case 2:
			test_str = "acc z";
			break;
			case 3:
			test_str = "gyro x";
			break;
			case 4:
			test_str = "acc y";
			break;
			case 5:
			test_str = "acc z";
			break;
		}
		print("MPU Reading %s: %d\n",test_str,MPU9250_results[i]);
		
		if (MPU9250_results[i] > MPU9250_expected[i]){
			if((MPU9250_results[i] - MPU9250_expected[i]) >= MPU9250_err_margin) {
				print("Error in test MPU9250 number %d \n",i);
				pass = 0;
			}
			} else {
			if((MPU9250_results[i] - MPU9250_expected[i]) >= MPU9250_err_margin) {
				print("Error in test MPU9250 number %d \n",i);
				pass = 0;
			}
		}
	}

	
	//WILL THESE WORK???? 
	print("HMC5883L test========================================\n");
	float test4 = HMC5883L_test(); 
	print("HMC test: %d\n",test4); 
	
	//ADC out of commission
	print("TEMD6200 test========================================\n");
	uint16_t pd_tests[6];
	for (int i = 0; i < 6; i++){
		
		switch (i) {
			case 0:
			test_str = "PD_FLASH";
			break;
			case 1:
			test_str = "PD_SIDE1";
			break;
			case 2:
			test_str = "PD_SIDE2";
			break;
			case 3:
			test_str = "PD_ACCESS";
			break;
			case 4:
			test_str = "PD_TOP1";
			break;
			case 5:
			test_str = "PD_TOP2";
			break;
		}
		
		pd_tests[i] = TEMD6200_test(i,5);
		print("TEMD6200 test %s: %d \n",test_str, pd_tests[i]);
	}
	uint16_t test5 = TEMD6200_test(0,5);
	
	
	print("TCA9535 test========================================\n"); 
	uint16_t rs;
	enum status_code sc = TCA9535_test(&rs);
	print("TCA return status: ");
	print_error(sc); 
	print("TCA test: %d\n",rs);
	
}
