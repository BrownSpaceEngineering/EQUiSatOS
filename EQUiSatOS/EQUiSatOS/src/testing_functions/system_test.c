/*
 * system_test.c
 *
 * Created: 10/22/2017 1:15:00 PM
 *  Author: Jarod Boone
 */ 
#include "system_test.h"

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

static char* get_panel(int panel_addr){
		////Flight IR Sensors
		//#define MLX90614_FLASHPANEL_V6_2_1	0x6C
		//#define MLX90614_TOPPANEL_V4_2		0x6B //
		//#define MLX90614_TOPPANEL_V4_1		0x6A
		//#define MLX90614_ACCESSPANEL_V4_1	0x5C
		//#define MLX90614_SIDEPANEL_V4_2		0x5D //
		//#define MLX90614_SIDEPANEL_V4_3		0x5F
		//#define MLX90614_SIDEPANEL_V4_4		0x6D
	switch (panel_addr){
		case 0x6C:
		return "MLX90614_FLASHPANEL_V6_2_1";
		
		case 0x6B:
		return "MLX90614_TOPPANEL_V4_2";
		
		case 0x6A:
		return "MLX90614_TOPPANEL_V4_1";
		
		case 0x5C:
		return "MLX90614_ACCESSPANEL_V4_1";
		
		case 0x5D: 
		return "MLX90614_SIDEPANEL_V4_2";
		
		case 0x5F: 
		return "MLX90614_SIDEPANEL_V4_3";
		
		case 0x6D:
		return "MLX90614_SIDEPANEL_V4_4";
		
		default: 
		return "INVALID FLASH PANEL ADDRESS";
	}
	
}

static float MLX90614_test(uint8_t addr){
	 
	 //configure_i2c_standard(SERCOM4); //SERCOM4 -> I2C serial port
	 
	 //DON'T NEED TO TURN ON FOR TEST BOARD. UNCOMMENT WHEN REAL TESTING
	 //MLX90614_init(); //Turns on the IR sensor and sets up the Port DO THIS IN MAIN()
	 
	 //Read -> return_struct_float
	 return_struct_16 rs;
	 MLX90614_read_all_obj(addr,rs);
	 
	 print("MLX90614 status: ");
	 print_error(rs.return_status); 
	 
	 //If return status is return status is category OK
	 if ((rs.return_status & 0x0F) != 0){
		 // return_value = NULL;
	 }
	 
	 return rs.return_value; 
}

static uint16_t AD590_test(int channel, int num_samples){
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
	return_struct_8 rs;
	LTC1380_channel_select(0x48, channel, rs);
	
	uint16_t sum =0;
	
	for (int j=0; j<num_samples; j++){
		adc_enable(&temp_instance);
		sum = sum +read_adc(temp_instance);
	}
	
	return sum/num_samples;	 
}

//IMU test
static void MPU9250_test(int16_t toFill[6]){
	//initalize imu.... probably
	MPU9250_init();
	
	float gyroBias[3] = {0, 0, 0}, accelBias[3] = {0, 0, 0};
	//MPU9250_computeBias(gyroBias,accelBias);
	enum status_code code1 = MPU9250_read_acc(toFill[0]);
	print("MPU9250 read accelerometer");
	print_error(code1);
	enum status_code code2 = MPU9250_read_gyro(toFill[3]);
	print("MPU9250 read gyroscope");
	print_error(code2); 
	
}

//Magnetometer test 
float HMC5883L_test(){
	
	uint8_t *buffer; 
	int16_t xyz[3]; 
	
	HMC5883L_init(); 
	HMC5883L_read(buffer); 
	HMC5883L_getXYZ(buffer, xyz); 
	return HMC5883L_computeCompassDir(xyz[0],xyz[1],xyz[2]); 
	
}

//GPIO test 
void TCA9535_test(return_struct_16 rs){
	
	TCA9535_init(&rs);
	
}

// Photodiode test 
uint16_t TEMD6200_test(int num_samples){
	
	struct adc_module pd_instance; 
	
	configure_adc(&pd_instance,P_AI_PD_OUT);
	return_struct_8 rs;
	LTC1380_channel_select(0x4a, 1, rs);
	//pdBuffer[i] =(readVoltagemV(pd_instance));//-6.5105)/0.3708; // I = exp((V-6.5105/0.3708)) in uA
			 
	uint16_t sum =0;
			 
	for (int j=0; j<num_samples; j++){
		adc_enable(&pd_instance);
		sum = sum +read_adc(pd_instance);
	}
	
	return sum/num_samples;

		
}



void AD7991_control_test_all(float *results_f){
	
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
	uint16_t temps[4], AD590_expected[] = 
	{expected_temp, expected_temp, expected_temp, expected_temp};
	
	
	//ADC out of comission at the moment
	//temps[0] = AD590_test(1, 1);
	//temps[1] = AD590_test(2, 5);
	//temps[2] = AD590_test(3, 5);
	//temps[3] = AD590_test(4, 5);
	
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
	print("IR test on %s yielded approx %d degrees Celsius\n",get_panel(MLX90614_ACCESSPANEL_V4_1),(int) test1);
	//float test2 = MLX90614_test(MLX90614_SIDEPANEL_V4_2);
	
	
	
	print("MPU9250 test========================================\n");
	print("Accelerometer readings in g, gyroscope readings in degrees per second");
	// IMU 
	// testmap 0 - acc x : 1 - acc y : 2 - acc z : 3 - gyr x : 4 - gyr y : 5 - gyr z
	uint16_t MPU9250_results[6], MPU9250_err_margin = 20; 
	MPU9250_test(MPU9250_results);
	uint16_t MPU9250_expected[] = {0, 0, 0, 0, 0, 0};
	char * a; 	
	for (int i = 0; i < 6; i++){
		
		switch (i) {
			case 0:
			a = "acc x";
			break;
			case 1:
			a = "acc y";
			break;
			case 2:
			a = "acc z";
			break;
			case 3:
			a = "gyro x";
			break;
			case 4:
			a = "gyro y";
			break;
			case 5:
			a = "gyro z";
			break;
		}
		print("MPU Reading %s: %d\n",a,MPU9250_results[i]);
		
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
	
	//ADC out of comission
	//uint16_t test5 = TEMD6200_test(5);
	
	
	print("TCA9535 test========================================\n"); 
	return_struct_16 rs;
	TCA9535_test(rs);
	print("TCA return status: ");
	print_error(rs.return_status); 
	print("TCA test: %d\n",rs.return_value);
	
}
