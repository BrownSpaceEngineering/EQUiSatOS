#include "flatsat.h"

struct adc_module temp_instance;
uint8_t magReadBuff[6] = {0, 0, 0, 0, 0, 0};
int16_t xyzBuff[3] = {0, 0, 0};
	
void flatsat_init(void) {
	configure_i2c_standard(SERCOM4);
	//MPU9250_init(i2c_write_command,i2c_read_command,i2c_write_command_no_stop);
	//while(true){
		//MPU9250_read();
	//}
	configure_pwm();
	delay_init();
	//MLX90614_init();
	//HMC5883L_init(*i2c_write_command, *i2c_read_command);	
	//configure_adc(&temp_instance,ADC_POSITIVE_INPUT_PIN8);
	//setup_pin(true,PIN_PA16); //init gpio pin for flashing
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
	set_output(true, PIN_PA16);
	delay_ms(100);
	set_output(false, PIN_PA16);
	delay_ms(900);
	set_output(true, PIN_PA16);
	delay_ms(100);
	set_output(false, PIN_PA16);
	delay_ms(900);
	set_output(true, PIN_PA16);
	delay_ms(100);
	set_output(false, PIN_PA16);
	delay_ms(900);
}

float read_temp(void) {
	float x = readVoltagemV(temp_instance);
	return x;
}

void flatsat_run(void) {	
	flatsat_init();
	while (1) {
		delay_ms(3000);
		setPulseWidthFraction(3,4);
		//read_IR();	
		//read_temp();
		//read_IMU();
		//led_flash();
	}
}