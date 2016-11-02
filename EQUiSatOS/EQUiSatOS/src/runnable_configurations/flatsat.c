#include "run.h"
#include "flatsat.h"

struct adc_module temp_instance;
uint8_t magReadBuff[6] = {0, 0, 0, 0, 0, 0};
int16_t xyzBuff[3] = {0, 0, 0};
	
void flatsat_init(void) {
	configure_i2c_master(SERCOM4);
	delay_init();
	
	MPU9250_init(i2c_write_command,i2c_read_command,i2c_write_command_no_stop);
	MPU9250Reading res = MPU9250_read();
	
	MLX90614_init(i2c_write_command,i2c_read_command,i2c_write_command_no_stop);
	HMC5883L_init(i2c_write_command, i2c_read_command);	
	//configure_adc(&temp_instance,ADC_POSITIVE_INPUT_PIN8);
	setup_pin(true,PIN_PA12); //init gpio pin for flashing
}

void read_IR(void) {
	float data = MLX90614_readTempC(MLX90614_DEFAULT_I2CADDR,false);
	//float data = MLX90614_readTempC(MLX90614_TBOARD_IR2,false);
}

void read_mag(void) {
	HMC5883L_read(magReadBuff);
	HMC5883L_getXYZ(magReadBuff, xyzBuff);
	float heading = HMC5883L_computeCompassDir(xyzBuff[0], xyzBuff[1], xyzBuff[2]);
}

void led_flash(void) {
	set_output(true, PIN_PA12);
	delay_ms(100);
	set_output(false, PIN_PA12);
	delay_ms(900);
	set_output(true, PIN_PA12);
	delay_ms(100);
	set_output(false, PIN_PA12);
	delay_ms(900);
	set_output(true, PIN_PA12);
	delay_ms(100);
	set_output(false, PIN_PA12);
	delay_ms(900);
}

void read_temp(void) {
	int x = readVoltagemV(temp_instance);
}

void flatsat_run(void) {	
	flatsat_init();
	while (1) {
		read_IR();
		read_mag();		
		led_flash();	
		//read_temp();
	}
}