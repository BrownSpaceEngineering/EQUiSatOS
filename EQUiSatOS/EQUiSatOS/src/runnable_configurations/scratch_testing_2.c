#include "scratch_testing_2.h"

struct adc_module temp_instance;
uint8_t magReadBuff[6] = {0, 0, 0, 0, 0, 0};
int16_t xyzBuff[3] = {0, 0, 0};
	
void flatsat_init(void) {

	configure_i2c_standard(SERCOM2);
	//MPU9250_init(i2c_write_command,i2c_read_command,i2c_write_command_no_stop);
	//while(true){
		//MPU9250_read();
	//}
	//configure_pwm();
	//delay_init();
	//MLX90614_init();
	//HMC5883L_init(*i2c_write_command, *i2c_read_command);	
	//configure_adc(&temp_instance,ADC_POSITIVE_INPUT_PIN8);
	//setup_pin(true,PIN_PB17);
	//setup_pin(true,PIN_PA12);
	//setup_pin(true,PIN_PB11);
	//set_output(true, PIN_PA12); //init gpio pin for flashing
	//configure_watchdog();
	//LTC1380_init();
	///setup_switching();
	TCA9535_init();
}

float read_IR(void) {
	float data = MLX90614_readTempC(MLX90614_DEFAULT_I2CADDR,OBJ1);
	//float data = MLX90614_readTempC(MLX90614_TBOARD_IR2,OBJ1);
	return data;
}

MPU9250Reading read_IMU(void) {
	MPU9250Reading data = MPU9250_read();
	return data;
}

void led_flash(void) {
	set_output(false, PIN_PA12);
	delay_ms(10);
	set_output(true, PIN_PA12);
	delay_ms(990);
	set_output(false, PIN_PA12);
	delay_ms(10);
	set_output(true, PIN_PA12);
	delay_ms(990);
	set_output(false, PIN_PA12);
	delay_ms(100);
	set_output(true, PIN_PA12);
	delay_ms(900);
}

float read_temp(void) {
	float x = readVoltagemV(temp_instance);
	return x;
}

void flatsat_run(void) {	
	flatsat_init();
	//set_output(true, PIN_PB11);
	//set_output(true, PIN_PB17);

	//while(true){
		//led_flash();
		//uint16_t x = AD7991_read(0x0);
		//int y = 2;
	//}
	
	
	/*
	pick_side(true);
	pick_input(0x1);
	struct adc_module temp_instance;
	configure_adc(&temp_instance,ADC_POSITIVE_INPUT_PIN8);
	
	while(true){
		float x = readVoltagemV(temp_instance);
		int y = 0;
	}
	*/
	/*
	if(did_watchdog_reset()){
		set_output(true,PIN_PB06);
	}
	
	for (int i = 0; i<20; i++)
	{
		reset_watchdog();
		delay_ms(500);
	}
	*/
	
	
	//while (1) {
		//delay_ms(3000);
		//setPulseWidthFraction(3,4);
		//read_IR();	
		//read_temp();
		//read_IMU();
		//led_flash();
		
	//}
	
	//LTC1380_channel_select(0x5);
	//LTC1380_disable();
	//LTC1380_channel_select(0x6);
	//LTC1380_disable();
	while(true){
		uint16_t x = readTCA9535Levels();
		
		int y = 0;
	}
}