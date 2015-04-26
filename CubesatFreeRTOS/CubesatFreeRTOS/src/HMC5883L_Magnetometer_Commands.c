/*
 * HMC5883L_Magnetometer_Commands.c
 *
 * Created: 4/26/2015 3:07:00 PM
 *  Author: Daniel
 */ 

#include <HMC5883L_Magnetometer_Commands.h>

/**
	Example for utilizing magnetometer with I2C
	DEPRECATED: Former main Used to test magnetometer when this was a separate project
*/
/*
int Example(void){
	//initialize SAMD21
	system_init();
	
	//configure PUTTY terminal
	configure_console();
	printf("Running...\n\r");
	
	//configure i2c master on SAMD21
	configure_i2c_master();
	printf("I2C Configured\n\r");
	
	//initialize HMC5883L magnetometer with standard settings
	HMC5883L_init();
	printf("HMC5883L (magnetometer) initialized\n\r");	
	
	Establish buffer to read from
	static uint8_t HMC5883L_read_buffer[7] = {
		'a','a','a','a','a','a',0x0
	};
	
	//take 10 measurements
	int i = 0;
	while(i < 10){
		HMC5883L_read(HMC5883L_read_buffer);
		printf("measurement\r\n");
		printf("%x%x%x%x%x%x\r\n",(char)HMC5883L_read_buffer[0],(char)HMC5883L_read_buffer[0],(char)HMC5883L_read_buffer[0],(char)HMC5883L_read_buffer[0],(char)HMC5883L_read_buffer[0],(char)HMC5883L_read_buffer[0]);
		i++;
	}
	
	
	printf("going to write dir command\r\n");
	
	i2c_write_command(&mc_dir_packet);
	printf("wrote dir command\r\n");
	//i2c_write_command(&mc_move_packet);
	//printf("wrote move command\r\n");
}
*/

/*
	Perform a standard initialization of the HMC5883L Magnetometer
	CURRENT SETTINGS:
	15HZ default measurement rate
	Gain = 5
	Single Measurement Mode
*/
void HMC5883L_init(void){
	static uint8_t write_buffer_1[2] = {
		0x00, 0x70
	};

	static uint8_t write_buffer_2[2] = {
		0x01, 0xA0
	};
	
	struct i2c_master_packet write_packet_1 = {
		.address     = HMC5883L_ADDRESS,
		.data_length = 2,
		.data        = write_buffer_1,
		.ten_bit_address = false,
		.high_speed      = false,
		.hs_master_code  = 0x0,
	};
	struct i2c_master_packet write_packet_2 = {
		.address     = HMC5883L_ADDRESS,
		.data_length = 2,
		.data        = write_buffer_2,
		.ten_bit_address = false,
		.high_speed      = false,
		.hs_master_code  = 0x0,
	};
	
	i2c_write_command(&write_packet_1);
	i2c_write_command(&write_packet_2);
}

/*
	Given an input buffer to read into (6 BYTES IN LENGTH MINIMUM) read from the x,y,z LSB and MSB registers
	on the HMC5883L magnetometer in single measurement mode
*/
void HMC5883L_read(uint8_t* read_buffer){
	struct i2c_master_packet read_packet = {
		.address     = HMC5883L_ADDRESS,
		.data_length = 6,
		.data        = read_buffer,
		.ten_bit_address = false,
		.high_speed      = false,
		.hs_master_code  = 0x0,
	};
	
	static uint8_t write_buffer[2] = {
		0x02, 0x01
	};
	
	struct i2c_master_packet write_packet = {
		.address     = HMC5883L_ADDRESS,
		.data_length = 2,
		.data        = write_buffer,
		.ten_bit_address = false,
		.high_speed      = false,
		.hs_master_code  = 0x0,
	};
	
	i2c_write_command(&write_packet);
	i2c_read_command(&read_packet);
}