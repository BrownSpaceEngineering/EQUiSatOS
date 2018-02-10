/*
 * battery_charging_tester.c
 *
 * Created: 11/2/2017 8:30:13 PM
 *  Author: rj16
 */

#include "battery_charging_tester.h"

// Note: we need the test flag to be set as true

#define LI_ABOVE_FULL              (LI_FULL_SANITY_MV + 50)
#define LI_BETWEEN_DOWN_AND_FULL   (LI_DOWN_MV + 25)
#define LI_BELOW_DOWN              (LI_DOWN_MV - 50)
#define LF_ABOVE_FULL							 (LF_FULL_MAX_MV + 10)

// The goals of testing are as follow:
//  - run single iterations of battery logic to assert its correctness in isolation
//    - the inputs, in general:
//      - voltages
//      - meta-state
//      - sub-state
//      - the battery that's charging
//      - the battery that's discharging
//      - the batteries that are decommissioned
//      - global state
//      - and sometimes...
//        - whether we've already set the satellite state
//        - timestamp
//    - the outputs, in general:
//      - meta-state
//      - sub-state
//      - the battery that's charging
//      - the battery that's discharging
//  - run many iterations of the battery logic to assert that over time, the battery
//    logic will converge to a good state (it will be a challenge to run this simulation
//    really well)

void test_init(void)
{
	init_charging_data();
	assert(charging_data.bat_charging == -1);
	assert(charging_data.lion_discharging == -1);
	assert(charging_data.curr_meta_charge_state == ALL_GOOD);
	assert(charging_data.curr_charge_state == FILL_LI_NEITHER_FULL_A);
	assert(charging_data.li_full_timestamp[0] == -1);
	assert(charging_data.li_full_timestamp[1] == -1);
	assert(charging_data.li_low_voltage_timestamp[0] == -1);
	assert(charging_data.li_low_voltage_timestamp[1] == -1);
	assert(charging_data.already_set_sat_state == 0);

	for (int i = 0; i < 4; i++)
	{
		assert(charging_data.bat_voltages[i] == -1);
		assert(charging_data.old_bat_voltages[i] == -1);
		assert(charging_data.decommissioned[i] == 0);
		assert(charging_data.decommissioned_timestamp[i] == -1);
		assert(charging_data.decommissioned_count[i] == 0);
		assert(charging_data.charging_parity == 0);
	}
}

void init_meta_test_charging_data(void)
{
	init_charging_data();
	charging_data.bat_charging = LI1;
	charging_data.lion_discharging = LI2;

	for (battery_t bat = 0; bat < 4; bat++)
		charging_data.bat_voltages[bat] = LI_ABOVE_FULL;
}

///
// between meta-states
///

void a_to_b(void)
{
	init_meta_test_charging_data();
	charging_data.decommissioned[LI1] = 1;
	battery_logic();

	assert(charging_data.curr_meta_charge_state == ONE_LI_DOWN);
	assert(charging_data.curr_charge_state == FILL_LI_B);
}

void a_to_c(void)
{
	init_meta_test_charging_data();
	charging_data.decommissioned[LI1] = 1;
	charging_data.decommissioned[LFB1] = 1;
	charging_data.decommissioned[LFB2] = 1;
	battery_logic();

	assert(charging_data.curr_meta_charge_state == TWO_LF_DOWN);
	assert(charging_data.curr_charge_state == FILL_LI_C);
}

void a_to_d(void)
{
	init_meta_test_charging_data();
	charging_data.decommissioned[LI1] = 1;
	charging_data.decommissioned[LI2] = 1;
	charging_data.decommissioned[LFB1] = 1;
	charging_data.decommissioned[LFB2] = 1;
	battery_logic();

	assert(charging_data.curr_meta_charge_state == TWO_LI_DOWN);
	assert(charging_data.curr_charge_state == FILL_LI_D);
}

void b_to_a(void)
{
	init_meta_test_charging_data();
	charging_data.curr_meta_charge_state = ONE_LI_DOWN;
	charging_data.curr_charge_state = FILL_LF_B;
	battery_logic();

	assert(charging_data.curr_meta_charge_state == ALL_GOOD);
	assert(charging_data.curr_charge_state == FILL_LI_NEITHER_FULL_A);
}

void b_to_c(void)
{
	init_meta_test_charging_data();
	charging_data.curr_meta_charge_state = ONE_LI_DOWN;
	charging_data.curr_charge_state = FILL_LF_B;
	charging_data.decommissioned[LI1] = 1;
	charging_data.decommissioned[LFB1] = 1;
	charging_data.decommissioned[LFB2] = 1;
	battery_logic();

	assert(charging_data.curr_meta_charge_state == TWO_LF_DOWN);
	assert(charging_data.curr_charge_state == FILL_LI_C);
}

void b_to_d(void)
{
	init_meta_test_charging_data();
	charging_data.curr_meta_charge_state = ONE_LI_DOWN;
	charging_data.curr_charge_state = FILL_LI_B;
	charging_data.decommissioned[LI1] = 1;
	charging_data.decommissioned[LI2] = 1;
	charging_data.decommissioned[LFB1] = 1;
	charging_data.decommissioned[LFB2] = 1;
	battery_logic();

	assert(charging_data.curr_meta_charge_state == TWO_LI_DOWN);
	assert(charging_data.curr_charge_state == FILL_LI_D);
}

void c_to_a(void)
{
	init_meta_test_charging_data();
	charging_data.curr_meta_charge_state = TWO_LF_DOWN;
	charging_data.curr_charge_state = FILL_LI_C;
	battery_logic();

	assert(charging_data.curr_meta_charge_state == ALL_GOOD);
	assert(charging_data.curr_charge_state == FILL_LI_NEITHER_FULL_A);
}

void c_to_b(void)
{
	init_meta_test_charging_data();
	charging_data.curr_meta_charge_state = TWO_LF_DOWN;
	charging_data.curr_charge_state = FILL_LI_C;
	charging_data.decommissioned[LI1] = 1;
	battery_logic();

	assert(charging_data.curr_meta_charge_state == ONE_LI_DOWN);
	assert(charging_data.curr_charge_state == FILL_LI_B);
}

void c_to_d(void)
{
	init_meta_test_charging_data();
	charging_data.curr_meta_charge_state = TWO_LF_DOWN;
	charging_data.curr_charge_state = FILL_LI_C;
	charging_data.decommissioned[LI1] = 1;
	charging_data.decommissioned[LI2] = 1;
	battery_logic();

	assert(charging_data.curr_meta_charge_state == TWO_LI_DOWN);
	assert(charging_data.curr_charge_state == FILL_LI_D);
}

void d_to_a(void)
{
	init_meta_test_charging_data();
	charging_data.curr_meta_charge_state = TWO_LI_DOWN;
	charging_data.curr_charge_state = FILL_LI_D;
	battery_logic();

	assert(charging_data.curr_meta_charge_state == ALL_GOOD);
	assert(charging_data.curr_charge_state == FILL_LI_NEITHER_FULL_A);
}

void d_to_b(void)
{
	init_meta_test_charging_data();
	charging_data.curr_meta_charge_state = TWO_LI_DOWN;
	charging_data.curr_charge_state = FILL_LI_D;
	charging_data.decommissioned[LI1] = 1;
	charging_data.decommissioned[LFB2] = 1;
	battery_logic();

	assert(charging_data.curr_meta_charge_state == ONE_LI_DOWN);
	assert(charging_data.curr_charge_state == FILL_LI_B);
}

void d_to_c(void)
{
	init_meta_test_charging_data();
	charging_data.curr_meta_charge_state = TWO_LI_DOWN;
	charging_data.curr_charge_state = FILL_LI_D;
	charging_data.decommissioned[LFB1] = 1;
	charging_data.decommissioned[LFB2] = 1;
	battery_logic();

	assert(charging_data.curr_meta_charge_state == TWO_LF_DOWN);
	assert(charging_data.curr_charge_state == FILL_LI_C);
}

void neither_full_to_same_a(void)
{
	init_charging_data();
	charging_data.bat_charging = LI1;
	charging_data.lion_discharging = LI2;
	charging_data.bat_voltages[LI1] = LI_BETWEEN_DOWN_AND_FULL - 1;
	charging_data.bat_voltages[LI2] = LI_BETWEEN_DOWN_AND_FULL + 1;
	simulated_state = IDLE_FLASH;
	battery_logic();

	assert(charging_data.curr_meta_charge_state == ALL_GOOD);
	assert(charging_data.curr_charge_state == FILL_LI_NEITHER_FULL_A);
}

void neither_full_to_l1_full_a(void)
{
	init_charging_data();
	charging_data.bat_charging = LI1;
	charging_data.lion_discharging = LI2;
	charging_data.bat_voltages[LI1] = LI_ABOVE_FULL;
	charging_data.bat_voltages[LI2] = LI_ABOVE_FULL + 2;
	simulated_state = IDLE_FLASH;
	simulated_curr_charging_filled_up = 1;
	battery_logic();

	assert(charging_data.curr_meta_charge_state == ALL_GOOD);
	assert(charging_data.curr_charge_state == FILL_LI_LI1_FULL_A);
}

void neither_full_to_l2_full_a(void)
{
	init_charging_data();
	charging_data.bat_charging = LI2;
	charging_data.lion_discharging = LI1;
	charging_data.bat_voltages[LI1] = LI_ABOVE_FULL + 2;
	charging_data.bat_voltages[LI2] = LI_ABOVE_FULL;
	simulated_state = IDLE_FLASH;
	simulated_curr_charging_filled_up = 1;
	battery_logic();

	assert(charging_data.curr_meta_charge_state == ALL_GOOD);
	assert(charging_data.curr_charge_state == FILL_LI_LI2_FULL_A);
}

void l1_full_to_same_a(void)
{
	init_charging_data();
	charging_data.curr_charge_state = FILL_LI_LI1_FULL_A;
	charging_data.bat_charging = LI2;
	charging_data.lion_discharging = LI1;
	charging_data.bat_voltages[LI1] = LI_ABOVE_FULL - 10;
	charging_data.bat_voltages[LI2] = LI_ABOVE_FULL - 60;
	simulated_state = IDLE_NO_FLASH;
	simulated_curr_charging_filled_up = 0;
	battery_logic();

	assert(charging_data.curr_meta_charge_state == ALL_GOOD);
	assert(charging_data.curr_charge_state == FILL_LI_LI1_FULL_A);
}

// NOTE: this also tests that going backwards takes precedence
void l1_full_to_neither_full_a(void)
{
	init_charging_data();
	charging_data.curr_charge_state = FILL_LI_LI1_FULL_A;
	charging_data.bat_charging = LI2;
	charging_data.lion_discharging = LI1;
	charging_data.bat_voltages[LI1] = LI_BETWEEN_DOWN_AND_FULL;
	charging_data.bat_voltages[LI2] = LI_ABOVE_FULL;
	simulated_state = IDLE_NO_FLASH;
	simulated_curr_charging_filled_up = 1;
	battery_logic();

	assert(charging_data.curr_meta_charge_state == ALL_GOOD);
	assert(charging_data.curr_charge_state == FILL_LI_NEITHER_FULL_A);
}

void l1_full_to_lf_a(void)
{
	init_charging_data();
	charging_data.curr_charge_state = FILL_LI_LI1_FULL_A;
	charging_data.bat_charging = LI2;
	charging_data.lion_discharging = LI1;
	charging_data.bat_voltages[LI1] = LI_ABOVE_FULL;
	charging_data.bat_voltages[LI2] = LI_ABOVE_FULL;
	simulated_state = IDLE_NO_FLASH;
	simulated_curr_charging_filled_up = 1;
	battery_logic();

	assert(charging_data.curr_meta_charge_state == ALL_GOOD);
	assert(charging_data.curr_charge_state == FILL_LF_A);
}

void l2_full_to_same_a(void)
{
	init_charging_data();
	charging_data.curr_charge_state = FILL_LI_LI2_FULL_A;
	charging_data.bat_charging = LI1;
	charging_data.lion_discharging = LI2;
	charging_data.bat_voltages[LI1] = LI_ABOVE_FULL - 60;
	charging_data.bat_voltages[LI2] = LI_ABOVE_FULL - 10;
	simulated_state = IDLE_NO_FLASH;
	simulated_curr_charging_filled_up = 0;
	battery_logic();

	assert(charging_data.curr_meta_charge_state == ALL_GOOD);
	assert(charging_data.curr_charge_state == FILL_LI_LI2_FULL_A);
}

// NOTE: this also tests that going backwards takes precedence
void l2_full_to_neither_full_a(void)
{
	init_charging_data();
	charging_data.curr_charge_state = FILL_LI_LI2_FULL_A;
	charging_data.bat_charging = LI1;
	charging_data.lion_discharging = LI2;
	charging_data.bat_voltages[LI1] = LI_ABOVE_FULL;
	charging_data.bat_voltages[LI2] = LI_BETWEEN_DOWN_AND_FULL;
	simulated_state = IDLE_NO_FLASH;
	simulated_curr_charging_filled_up = 1;
	battery_logic();

	assert(charging_data.curr_meta_charge_state == ALL_GOOD);
	assert(charging_data.curr_charge_state == FILL_LI_NEITHER_FULL_A);
}

void l2_full_to_lf_a(void)
{
	init_charging_data();
	charging_data.curr_charge_state = FILL_LI_LI2_FULL_A;
	charging_data.bat_charging = LI1;
	charging_data.lion_discharging = LI2;
	charging_data.bat_voltages[LI1] = LI_ABOVE_FULL;
	charging_data.bat_voltages[LI2] = LI_ABOVE_FULL;
	simulated_state = IDLE_NO_FLASH;
	simulated_curr_charging_filled_up = 1;
	battery_logic();

	assert(charging_data.curr_meta_charge_state == ALL_GOOD);
	assert(charging_data.curr_charge_state == FILL_LF_A);
}

void lf_to_same_a(void)
{
	init_charging_data();
	charging_data.curr_charge_state = FILL_LF_A;
	charging_data.bat_charging = LFB2;
	charging_data.lion_discharging = LI1;
	charging_data.bat_voltages[LI1] = LI_ABOVE_FULL + 2;
	charging_data.bat_voltages[LI2] = LI_ABOVE_FULL;
	simulated_state = IDLE_NO_FLASH;
	battery_logic();

	assert(charging_data.curr_meta_charge_state == ALL_GOOD);
	assert(charging_data.curr_charge_state == FILL_LF_A);
}

void lf_to_neither_full_a(void)
{
	init_charging_data();
	charging_data.curr_charge_state = FILL_LF_A;
	charging_data.bat_charging = LFB2;
	charging_data.lion_discharging = LI1;
	charging_data.bat_voltages[LI1] = LI_BETWEEN_DOWN_AND_FULL;
	charging_data.bat_voltages[LI2] = LI_BELOW_DOWN;
	simulated_state = IDLE_NO_FLASH;
	battery_logic();

	assert(charging_data.curr_meta_charge_state == ALL_GOOD);
	assert(charging_data.curr_charge_state == FILL_LI_NEITHER_FULL_A);
}

void to_neither_full_for_global_state_a(void)
{
	init_charging_data();
	charging_data.curr_charge_state = FILL_LF_A;
	charging_data.bat_charging = LFB2;
	charging_data.lion_discharging = LI1;
	charging_data.bat_voltages[LI1] = LI_ABOVE_FULL + 2;
	charging_data.bat_voltages[LI2] = LI_ABOVE_FULL;
	simulated_state = LOW_POWER; // this is a bit of a construed example
	battery_logic();

	assert(charging_data.curr_meta_charge_state == ALL_GOOD);
	assert(charging_data.curr_charge_state == FILL_LI_NEITHER_FULL_A);
}

void to_neither_full_for_lf_full_a(void)
{
	init_charging_data();
	charging_data.curr_charge_state = FILL_LF_A;
	charging_data.bat_charging = LFB2;
	charging_data.lion_discharging = LI1;
	charging_data.bat_voltages[LI1] = LI_ABOVE_FULL + 2;
	charging_data.bat_voltages[LI2] = LI_ABOVE_FULL;
	charging_data.bat_voltages[LFB1] = LF_ABOVE_FULL;
	charging_data.bat_voltages[LFB2] = LF_ABOVE_FULL;
	simulated_state = IDLE_NO_FLASH;
	battery_logic();

	assert(charging_data.curr_meta_charge_state == ALL_GOOD);
	assert(charging_data.curr_charge_state == FILL_LI_NEITHER_FULL_A);
}

void li_to_same_b(void)
{
	init_charging_data();
	charging_data.curr_meta_charge_state = ONE_LI_DOWN;
	charging_data.curr_charge_state = FILL_LI_B;
	charging_data.decommissioned[LI2] = 1;
	charging_data.bat_charging = LI1;
	charging_data.lion_discharging = LI1;
	charging_data.bat_voltages[LI1] = LI_ABOVE_FULL;
	simulated_state = IDLE_NO_FLASH;
	simulated_curr_charging_filled_up = 0;
	battery_logic();

	assert(charging_data.curr_meta_charge_state == ONE_LI_DOWN);
	assert(charging_data.curr_charge_state == FILL_LI_B);
}

void li_to_lf_b(void)
{
	init_charging_data();
	charging_data.curr_meta_charge_state = ONE_LI_DOWN;
	charging_data.curr_charge_state = FILL_LI_B;
	charging_data.decommissioned[LI1] = 1;
	charging_data.decommissioned[LFB1] = 1;
	charging_data.bat_charging = LI2;
	charging_data.lion_discharging = LI2;
	charging_data.bat_voltages[LI2] = LI_ABOVE_FULL;
	simulated_state = IDLE_FLASH;
	simulated_curr_charging_filled_up = 1;
	battery_logic();

	assert(charging_data.curr_meta_charge_state == ONE_LI_DOWN);
	assert(charging_data.curr_charge_state == FILL_LF_B);
}

void lf_to_same_b(void)
{
	init_charging_data();
	charging_data.curr_meta_charge_state = ONE_LI_DOWN;
	charging_data.curr_charge_state = FILL_LF_B;
	charging_data.decommissioned[LI1] = 1;
	charging_data.decommissioned[LFB1] = 1;
	charging_data.bat_charging = LFB2;
	charging_data.lion_discharging = LI2;
	charging_data.bat_voltages[LI1] = LI_BETWEEN_DOWN_AND_FULL;
	charging_data.bat_voltages[LI2] = LI_ABOVE_FULL;
	simulated_state = IDLE_FLASH;
	battery_logic();

	assert(charging_data.curr_meta_charge_state == ONE_LI_DOWN);
	assert(charging_data.curr_charge_state == FILL_LF_B);
}

void lf_to_li_b(void)
{
	init_charging_data();
	charging_data.curr_meta_charge_state = ONE_LI_DOWN;
	charging_data.curr_charge_state = FILL_LF_B;
	charging_data.decommissioned[LI1] = 1;
	charging_data.decommissioned[LFB1] = 1;
	charging_data.bat_charging = LFB2;
	charging_data.lion_discharging = LI2;
	charging_data.bat_voltages[LI1] = LI_BELOW_DOWN;
	simulated_state = IDLE_FLASH;
	battery_logic();

	assert(charging_data.curr_meta_charge_state == ONE_LI_DOWN);
	assert(charging_data.curr_charge_state == FILL_LI_B);
}

void to_li_for_global_state_b(void)
{
	init_charging_data();
	charging_data.curr_meta_charge_state = ONE_LI_DOWN;
	charging_data.curr_charge_state = FILL_LF_B;
	charging_data.decommissioned[LI1] = 1;
	charging_data.decommissioned[LFB1] = 1;
	charging_data.bat_charging = LFB2;
	charging_data.lion_discharging = LI2;
	charging_data.bat_voltages[LI1] = LI_BETWEEN_DOWN_AND_FULL;
	simulated_state = LOW_POWER;
	battery_logic();

	assert(charging_data.curr_meta_charge_state == ONE_LI_DOWN);
	assert(charging_data.curr_charge_state == FILL_LI_B);
}

void to_li_for_lf_full_b(void)
{
	init_charging_data();
	charging_data.curr_meta_charge_state = ONE_LI_DOWN;
	charging_data.curr_charge_state = FILL_LF_B;
	charging_data.decommissioned[LI1] = 1;
	charging_data.decommissioned[LFB1] = 1;
	charging_data.bat_charging = LFB2;
	charging_data.lion_discharging = LI2;
	charging_data.bat_voltages[LI1] = LI_BETWEEN_DOWN_AND_FULL;
	charging_data.bat_voltages[LFB2] = LF_ABOVE_FULL;
	simulated_state = IDLE_NO_FLASH;
	battery_logic();

	assert(charging_data.curr_meta_charge_state == ONE_LI_DOWN);
	assert(charging_data.curr_charge_state == FILL_LI_B);
}

void c_to_c(void)
{
	init_charging_data();
	charging_data.curr_meta_charge_state = TWO_LF_DOWN;
	charging_data.curr_charge_state = FILL_LI_C;
	charging_data.decommissioned[LFB1] = 1;
	charging_data.decommissioned[LFB2] = 1;
	charging_data.bat_charging = LI1;
	charging_data.lion_discharging = LI2;
	charging_data.bat_voltages[LI1] = LI_BETWEEN_DOWN_AND_FULL;
	charging_data.bat_voltages[LI2] = LI_BETWEEN_DOWN_AND_FULL;
	simulated_state = LOW_POWER;
	battery_logic();

	assert(charging_data.curr_meta_charge_state == TWO_LF_DOWN);
	assert(charging_data.curr_charge_state == FILL_LI_C);
}

void d_to_d(void)
{
	init_charging_data();
	charging_data.curr_meta_charge_state = TWO_LI_DOWN;
	charging_data.curr_charge_state = FILL_LI_D;
	charging_data.decommissioned[LI1] = 1;
	charging_data.decommissioned[LI2] = 1;
	charging_data.decommissioned[LFB1] = 1;
	charging_data.decommissioned[LFB2] = 1;
	charging_data.bat_charging = LI1;
	charging_data.lion_discharging = LI2;
	charging_data.bat_voltages[LI1] = LI_BETWEEN_DOWN_AND_FULL;
	charging_data.bat_voltages[LI2] = LI_BETWEEN_DOWN_AND_FULL;
	simulated_state = LOW_POWER;
	battery_logic();

	assert(charging_data.curr_meta_charge_state == TWO_LI_DOWN);
	assert(charging_data.curr_charge_state == FILL_LI_D);
}


void run_unit_tests(void)
{
	test_init();

	///
	// changes in the meta-state -- dictated by the number of decommissioned
	// batteries
	///

	a_to_b(); 
	a_to_c(); 
	a_to_d();
	b_to_a(); 
	b_to_c(); 
	b_to_d();
	c_to_a(); 
	c_to_b(); 
	c_to_d();
	d_to_a(); 
	d_to_b(); 
	d_to_c();

	///
	// ALL_GOOD (A)
	///

	// from neither_full
	neither_full_to_same_a();
	neither_full_to_l1_full_a();
	neither_full_to_l2_full_a();

	// from l1_full
	l1_full_to_same_a();
	l1_full_to_neither_full_a();
	l1_full_to_lf_a();

	// from l2_full
	l2_full_to_same_a();
	l2_full_to_neither_full_a();
	l2_full_to_lf_a();

	// from lf
	lf_to_same_a();
	lf_to_neither_full_a();

	// back to neither_full
	to_neither_full_for_global_state_a();
	to_neither_full_for_lf_full_a();

	///
	// ONE_LI_DOWN (B)
	///

	li_to_same_b();
	li_to_lf_b();

	lf_to_same_b();
	lf_to_li_b();

	to_li_for_global_state_b();
	to_li_for_lf_full_b();

	///
	// other two
	///

	c_to_c();
	d_to_d();
}

void run_simulations(void)
{
	// TODO: fill this out
}

void run_battery_charging_tests(void)
{
	run_unit_tests();
	run_simulations();
}
