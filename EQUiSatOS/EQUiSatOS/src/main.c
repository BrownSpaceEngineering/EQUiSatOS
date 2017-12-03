//#include "runnable_configurations/flatsat.h"
#include "main.h"
#include "runnable_configurations/flatsat.h"
#include "runnable_configurations/bat_testing.h"

static void run_tests(void) {
	//sensor_read_tests();
	//flatsat_run(); //calls flatsat init
	//bat_testing_run();
	//test_equistack();
	//assert_rtos_constants();
	//assert_transmission_constants();

	//system_test(); 
	/************************************************************************/
	/* AD HOC TESTS GO AFTER HERE - Make a FUNCTION for them                */
	/************************************************************************/
	//The_Fall_Of_Ryan();

	/*configure_pwm(P_ANT_DRV1, P_ANT_DRV1_MUX);
	init_tc();
	while(get_count() < 3) {
			set_pulse_width_fraction(3, 4);
	}
	disable_pwm();
	*/
}

static void sensor_read_tests(void) {
	uint16_t six_buf[6];
	uint8_t six_buf_8t[6];
	uint16_t three_buf[3];
	uint16_t four_buf[4];
	uint16_t two_buf[2];
	delay_ms(100);
	print("\n\n\n\n##### NEW RUN #####\n");
	
	print("\n# IMU #\n");
	read_accel_batch(three_buf);
	print("accel: %d %d %d\n", three_buf[0], three_buf[1], three_buf[2]);
	read_gyro_batch(three_buf);
	print("gyro: %d %d %d\n", three_buf[0], three_buf[1], three_buf[2]);
	read_magnetometer_batch(three_buf);
	print("magnetometer: %d %d %d\n", three_buf[0], three_buf[1], three_buf[2]);
	
	print("\n# IR #\n");
	read_ir_ambient_temps_batch(six_buf);
	print("ir ambs: %d %d\n", (uint16_t)dataToTemp(six_buf[1]), (uint16_t)dataToTemp(six_buf[2]));
	read_ir_object_temps_batch(six_buf);
	print("ir objs: %d %d\n", (uint16_t)dataToTemp(six_buf[1]), (uint16_t)dataToTemp(six_buf[2]));
	
	print("\n# PDIODE #\n");
	read_pdiode_batch(six_buf_8t);
	for (int i = 0; i < 6; i++){
		print("pdiode %d: %d\n",i, six_buf_8t[i]);	
	}
	
	print("\n\n# LiON VOLTS #\n");
	read_lion_volts_batch(two_buf);
	print("lion volts: %d %d\n", two_buf[0], two_buf[1]);
	
	print("# LiON CURRENT #\n");
	read_lion_current_batch(two_buf);
	print("lion current: %d %d\n", two_buf[0], two_buf[1]);
	
	print("# LiON TEMPS #\n");
	read_lion_temps_batch(two_buf);
	print("lion temps: %d %d\n", two_buf[0], two_buf[1]);
	
	
	print("\n# LiFePO VOLTS #\n");
	read_lifepo_volts_batch(four_buf);
	print("lifepo volts: %d %d %d %d\n", four_buf[0], four_buf[1], four_buf[2], four_buf[3]);
	
	print("# LiFePO CURRENT #\n");
	read_lifepo_current_batch(four_buf);
	print("lifepo current: %d %d %d %d\n", four_buf[0], four_buf[1], four_buf[2], four_buf[3]);
}

int main(void)
{
	global_init();
	/*print("BEGIN COUNT: %d\n", get_rtc_count());
	for (int i = 0; i < 200; i++) {
		flashBurstTest();	
		//print("%d\n", i);
	}
	print("END COUNT: %d\n", get_rtc_count());*/
	//sensor_read_tests();
	//system_test();
	setup_pin(true, P_RAD_PWR_RUN);
	setup_pin(true, P_RAD_SHDN);
	set_output(false, P_RAD_PWR_RUN);
	set_output(false, P_RAD_SHDN);
	
	//int x = 1; 
	//run_tests();
	//run_rtos();	
}
