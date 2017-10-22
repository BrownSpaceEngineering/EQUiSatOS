/*
 * system_test.c
 *
 * Created: 10/22/2017 1:15:00 PM
 *  Author: Jarod Boone
 */ 
/*

**ADC USE** 
void configure_adc(struct adc_module *adc_instance, enum adc_positive_input pin) {
	struct adc_config config_adc;
	// setup_config_defaults
	adc_get_config_defaults(&config_adc);

	config_adc.resolution = ADC_RESOLUTION_10BIT;

	config_adc.positive_input = pin;
	//setup_set_config
	adc_init(adc_instance, ADC, &config_adc);
	adc_enable(adc_instance);
}

//reads the current voltage from the ADC connection
float readVoltagemV(struct adc_module adc_instance) {
	if (!&adc_instance || !adc_instance.hw) {
		//You must configure the adc_instance and set it as a global variable.
		return -1;
	}
	
	uint16_t result = 0;
	int status;
	
	//start conversion
	adc_start_conversion(&adc_instance);
	
	do {
		// Wait for conversion to be done and read out result
		status = adc_read(&adc_instance, &result);
	} while (status == STATUS_BUSY);
	float resFloat = result;
	return resFloat;
}
*/ 
#define CTRL_BOARD 1

#include "system_test.h"

float MLX90614_test(uint8_t addr){
	 
	 configure_i2c_standard(SERCOM4); //SERCOM4 -> I2C serial port
	 
	 //DON'T NEED TO TURN ON FOR TEST BOARD. UNCOMMENT WHEN REAL TESTING
	 MLX90614_init(); //Turns on the IR sensor and sets up the Port
	 
	 //Read -> return_struct_float
	 struct return_struct_float IR_read = MLX90614_readTempC(addr,OBJ1);
	 float return_value = IR_read.return_value;
	 
	 //If return status is return status is category OK
	 if ((IR_read.return_status & 0x0F) != 0){
		 // return_value = NULL;
	 }
	 
	 return return_value; 
}

float AD590_test(){
	
	if(CTRL_BOARD) { //control board
		//To read CNTRLTEMP, put PB30 low, PB22 and PB23 High, PB10 low, and expect reading on PB00.

	} else {
		struct adc_module temp_instance; //generate object
		setup_pin(true, 51);
		set_output(true, 51);
		setup_pin(true, 49);
		set_output(true, 49);
		setup_pin(true, 50);
		set_output(false, 50);
		
		//Set up object to adc pin
		configure_adc(&temp_instance,ADC_POSITIVE_INPUT_PIN8); //IS THIS THE RIGHT PIN???? -> probably not lmao
		
		//try reading
		return readVoltagemV(temp_instance);
	}
	
	
}

MPU9250Reading MPU9250_test(){
	//initalize imu.... probably
	MPU9250_init();
	
	float gyroBias[3] = {0, 0, 0}, accelBias[3] = {0, 0, 0};
	MPU9250_computeBias(gyroBias,accelBias);
	MPU9250Reading reading;
}

float HMC5883L_test(){
	
	uint8_t *buffer; 
	int16_t xyz[3]; 
	
	HMC5883L_init(); 
	HMC5883L_read(buffer); 
	HMC5883L_getXYZ(buffer, xyz); 
	return HMC5883L_computeCompassDir(xyz[0],xyz[1],xyz[2]); 
	
}

return_struct_16 TCA9535_test(){
	
	struct return_struct_16 init = TCA9535_init();
	return readTCA9535Levels(); 
	
}

float TEMD6200_test(){
	//test the photodiode 
	
	//To read in LED_Sense put PA10 low, PB22 High, PB23 Low, and expect reading on PB00.
	

}



void system_test(void){
	
	//Test IR sensor
	float test1 = MLX90614_test(MLX90614_DEFAULT_I2CADDR);
	float test2 = AD590_test(); 
	MPU9250Reading test3 = MPU9250_test();
	float test4 = HMC5883L_test(); 
	return_struct_16 TCA9535_test(); 
	
}