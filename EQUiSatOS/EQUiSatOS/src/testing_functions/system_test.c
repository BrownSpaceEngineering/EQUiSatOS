/*
 * system_test.c
 *
 * Created: 10/22/2017 1:15:00 PM
 *  Author: Jarod Boone
 */ 
#include "system_test.h"

float MLX90614_test(uint8_t addr){
	 
	 //configure_i2c_standard(SERCOM4); //SERCOM4 -> I2C serial port
	 
	 //DON'T NEED TO TURN ON FOR TEST BOARD. UNCOMMENT WHEN REAL TESTING
	 MLX90614_init(); //Turns on the IR sensor and sets up the Port
	 
	 //Read -> return_struct_float
	 return_struct_16 rs;
	 MLX90614_read_all_obj(addr,rs);
	 
	 //If return status is return status is category OK
	 if ((rs.return_status & 0x0F) != 0){
		 // return_value = NULL;
	 }
	 
	 return rs.return_value; 
}

uint16_t AD590_test(int channel, int num_samples){

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
void MPU9250_test(int16_t toFill[6]){
	//initalize imu.... probably
	MPU9250_init();
	
	float gyroBias[3] = {0, 0, 0}, accelBias[3] = {0, 0, 0};
	//MPU9250_computeBias(gyroBias,accelBias);
	MPU9250_read_acc(toFill[0]);
	MPU9250_read_gyro(toFill[3]);
	
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
return_struct_16 TCA9535_test(){
	return_struct_16 rs;
	TCA9535_init(rs);
	readTCA9535Levels(rs); 
	return rs;
	
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

//input a uint8_t array of length 4, it will fill each index with the corresponding test
// channel 0 -> results[0],  channel 1 -> results[1] etc...
// 1 represents a failed channel test 0 represents a passed channel test
/*void AD7991_control_test(uint8_t *results){
	
	const uint16_t error_tolerance = 100; // how many millivolts off expected value will we accept?
	
	//expected_value_<channel> = expected value in millivolts of regulator on <channel>
	const uint16_t expected_value_0 = 3600;
	const uint16_t calibration_0 = 2020;
	
	const uint16_t expected_value_1 = 3600; 
	const uint16_t calibration_1 = 26;
	
	const uint16_t expected_value_2 = 5000; 
	const uint16_t calibration_2 = 3326;
	
	const uint16_t expected_value_3 = 3300; 
	const uint16_t calibration_3 = 2005; 
	
	const uint16_t reference_value = 3300 * 1000;
	const uint16_t total = 4096 * 1000; 
	
	uint16_t difference; 
	
	setup_pin(true, P_RAD_PWR_RUN); //3.6V regulator 
	setup_pin(true, P_5V_EN); // 5V regulator 
	
		
	set_output(true, P_RAD_PWR_RUN);		
	set_output(true, P_5V_EN);
	
	AD7991_change_channel(0x0); //Set channel to 0 -> 3.6V Regulator Voltage Sense
	uint16_t reading = AD7991_read(0x0); 
	uint32_t hr_read = (reading * reference_value * calibration_0)/total;
	
	//compute difference, if you have a better way to get abs difference between 2 
	//uint16_t without knowing which one is bigger pls email me jtboone12@gmail.com
	if (reading > expected_value_0) {
		difference = reading - expected_value_0;
	} else {
		difference = expected_value_0 - reading; 
	}
	
	//is the value acceptable? 
	if (difference > error_tolerance){
		results[0] = 1; 
	} else {
		results[0] = 0; 
	}
	
	AD7991_change_channel(0x1); //Set channel to 1 -> 3.6V Regulator Current Sense
	reading = AD7991_read(0x1); 
	reading = (reading * reference_value * calibration_1)/total; //turn reading into millivolts
	
	//compute difference, if you have a better way to get abs difference between 2 
	//uint16_t without knowing which one is bigger pls email me jtboone12@gmail.com
	if (reading > expected_value_1) {
		difference = reading - expected_value_1;
	} else {
		difference = expected_value_1 - reading; 
	}
	
	//is the value exceptable? 
	if (difference > error_tolerance){
		results[1] = 1; 
	} else {
		results[1] = 0; 
	}
	
	AD7991_change_channel(0x2); //Set channel to 1 -> 3.6V Regulator Current Sense
	reading = AD7991_read(0x2); 
	reading = (reading * reference_value * calibration_2)/total; //turn reading into millivolts
	
	//compute difference, if you have a better way to get abs difference between 2 
	//uint16_t without knowing which one is bigger pls email me jtboone12@gmail.com
	if (reading > expected_value_2) {
		difference = reading - expected_value_2;
	} else {
		difference = expected_value_2 - reading; 
	}
	
	//is the value exceptable? 
	if (difference > error_tolerance){
		results[2] = 1; 
	} else {
		results[2] = 0; 
	}
	
	AD7991_change_channel(0x3); //Set channel to 1 -> 3.6V Regulator Current Sense
	reading = AD7991_read(0x3); 
	reading = (reading * reference_value * calibration_3)/total; //turn reading into millivolts
	
	//compute difference, if you have a better way to get abs difference between 2 
	//uint16_t without knowing which one is bigger pls email me jtboone12@gmail.com
	if (reading > expected_value_3) {
		difference = reading - expected_value_3;
	} else {
		difference = expected_value_3 - reading; 
	}
	
	//is the value exceptable? 
	if (difference > error_tolerance){
		results[3] = 1; 
	} else {
		results[3] = 0; 
	}		
}*/

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
	
	MLX90614_init();
	configure_i2c_master(SERCOM4); //init I2C
	//LTC1380_init(); //init multiplexer

	float results[4]; 
	uint16_t expected[] = {3.6, 3.6, 5, 3.3}; 
	
	AD7991_control_test_all(results);
	int AD7991_test_results[4] = {0,0,0,0};
	
	//check with expected values
	for (int i = 0; i < 4; i++){
		if (results[i] > expected[i]){
			if((results[i] - expected[i]) >= 0.5) {
				AD7991_test_results[i] = 1;
			}
		} else {
			if((expected[i] - results[i]) >= 0.5) {
				AD7991_test_results[i] = 1;
			}
		}
	}
	
	
	uint16_t temps[4];
	
	temps[0] = AD590_test(1, 1);
	temps[1] = AD590_test(2, 5);
	temps[2] = AD590_test(3, 5);
	temps[3] = AD590_test(4, 5);
	
	////Flight IR Sensors
	//#define MLX90614_FLASHPANEL_V6_2_1	0x6C
	//#define MLX90614_TOPPANEL_V4_2		0x6B //
	//#define MLX90614_TOPPANEL_V4_1		0x6A
	//#define MLX90614_ACCESSPANEL_V4_1	0x5C
	//#define MLX90614_SIDEPANEL_V4_2		0x5D //
	//#define MLX90614_SIDEPANEL_V4_3		0x5F
	//#define MLX90614_SIDEPANEL_V4_4		0x6D
	float test1 = MLX90614_test(MLX90614_ACCESSPANEL_V4_1);
	//float test2 = MLX90614_test(MLX90614_SIDEPANEL_V4_2);
	
	// IMU 
	uint16_t fill[6]; 
	MPU9250_test(fill);
	
	float test4 = HMC5883L_test(); 
	uint16_t test5 = TEMD6200_test(5); 
	// return_struct_16 TCA9535_test(); 
	
}
