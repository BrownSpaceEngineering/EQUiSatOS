/*
 * intTesting.c
 *
 * Created: 3/24/2017 5:40:57 PM
 *  Author: rizant
 */ 
#include "intTesting.h"
#include "../config/proc_pins.h"

struct adc_module temp_instance;
uint8_t magReadBuff[6] = {0, 0, 0, 0, 0, 0};
int16_t xyzBuff[3] = {0, 0, 0};
struct adc_module led3sns_instance;
struct adc_module led4sns_instance;
	
void flatsat_init(void) {

	configure_i2c_standard(SERCOM4);
	//MPU9250_init(i2c_write_command,i2c_read_command,i2c_write_command_no_stop);
	//while(true){
		//MPU9250_read();
	//}
	//configure_pwm();
	delay_init();
	MLX90614_init();
	//HMC5883L_init(*i2c_write_command, *i2c_read_command);	
	//configure_adc(&temp_instance,ADC_POSITIVE_INPUT_PIN8);
	setup_pin(true,P_LED_CMD);
	setup_pin(true,P_5V_EN);
	//setup_pin(true,PIN_PB11);
	set_output(true, P_LED_CMD); //init gpio pin for flashing
	//configure_watchdog();
	//LTC1380_init();
	///setup_switching();
	TCA9535_init();
	configure_adc(&led3sns_instance,ADC_POSITIVE_INPUT_PIN9); //set up adc for led3sns
	configure_adc(&led4sns_instance,ADC_POSITIVE_INPUT_PIN8); // set up adc for led4sns
}

float read_IR(void) {
	float data = MLX90614_readTempC(MLX90614_FLASH_PANEL_B1,OBJ1);
	//float data = MLX90614_readTempC(MLX90614_TBOARD_IR2,OBJ1);
	return data;
}

MPU9250Reading read_IMU(void) {
	MPU9250Reading data = MPU9250_read();
	return data;
}


void led_flash(void) {
	
	set_output(false, P_LED_CMD);
	delay_ms(10);
	float val3_1 = readVoltagemV(led3sns_instance);
	float val4_1 = readVoltagemV(led4sns_instance);
	//float ir_1 = read_IR();
	
	set_output(true, P_LED_CMD);
	delay_ms(990);
	set_output(false, P_LED_CMD);
	delay_ms(10);
	float val3_2 = readVoltagemV(led3sns_instance);
	float val4_2 = readVoltagemV(led4sns_instance);
	//float ir_2 = read_IR();
	
	set_output(true, P_LED_CMD);
	delay_ms(990);
	set_output(false, P_LED_CMD);
	delay_ms(10);
	float val3_3 = readVoltagemV(led3sns_instance);
	float val4_3 = readVoltagemV(led4sns_instance);
	//float ir_3 =  read_IR();
	
	set_output(true, P_LED_CMD);
	delay_ms(990);
}

float read_temp(void) {
	float x = readVoltagemV(temp_instance);
	return x;
}

void integratedTesting_run(void) {	
	flatsat_init();
	//delay_ms(2000);
	float initial_led3 = readVoltagemV(led3sns_instance);
	float initial_led4 = readVoltagemV(led4sns_instance);
	
	set_output(true, P_5V_EN);
	int count = 0;
	while(count<5){
		led_flash();
		delay_ms(2000);
		//uint16_t x = AD7991_read(0x0);
		//int y = 2;
		count = count+1;
	}
	
	
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
	/*while(true){
		uint16_t x = readTCA9535Levels();
		
		int y = 0;
	}*/
}