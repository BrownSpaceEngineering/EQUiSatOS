/*
 * battery_charging_tester.h
 *
 * Created: 11/2/2017 8:30:26 PM
 *  Author: rj16
 */

#ifndef BATTERY_CHARGING_TESTER_H_
#define BATTERY_CHARGING_TESTER_H_

#include "rtos_tasks/battery_charging_task.h"
#include "equisim_simulated_data.h"

void set_voltages(int li1_mv, int li2_mv, int lf1_mv, int lf2_mv);
void run_battery_charging_tests(void);

void test_init(void);
void init_meta_test_charging_data(void);

///
// changes in the meta-state -- dictated by the number of decommissioned
// batteries
///

void a_to_b(void);
void a_to_c(void);
void a_to_d(void);
void b_to_a(void);
void b_to_c(void);
void b_to_d(void);
void c_to_a(void);
void c_to_b(void);
void c_to_d(void);
void d_to_a(void);
void d_to_b(void);
void d_to_c(void);

///
// ALL_GOOD (A)
///

// from neither_full
void neither_full_to_same_a(void);
void neither_full_to_l1_full_a(void);
void neither_full_to_l2_full_a(void);

// from l1_full
void l1_full_to_same_a(void);
void l1_full_to_neither_full_a(void);
void l1_full_to_lf_a(void);

// from l2_full
void l2_full_to_same_a(void);
void l2_full_to_neither_full_a(void);
void l2_full_to_lf_a(void);

// from lf
void lf_to_same_a(void);
void lf_to_neither_full_a(void);

// back to neither_full
void to_neither_full_for_global_state_a(void);
void to_neither_full_for_lf_full_a(void);

///
// ONE_LI_DOWN (B)
///

void li_to_same_b(void);
void li_to_lf_b(void);

void lf_to_same_b(void);
void lf_to_li_b(void);

void to_li_for_global_state_b(void);
void to_li_for_lf_full_b(void);

///
// other two
///

void c_to_c(void);
void d_to_d(void);

///
// checking decisions about batteries to charge and discharge, given a
// meta-state and a sub-state
///

void neither_full_li1_a(void);
void neither_full_li2_a(void);
void l1_full_a(void);
void l2_full_a(void);
void fill_lf_lf1_a(void);
void fill_lf_lf2_a(void);
void fill_li_li1_down_b(void);
void fill_li_li2_down_b(void);
void fill_lf_li2_down_lf1_b(void);
void fill_lf_li2_down_lf2_b(void);
void fill_lf_li2_down_lf1_down_b(void);
void fill_lf_li2_down_lf2_down_b(void);
void li1_c(void);
void li2_c(void);
void li1_down_c(void);
void li2_down_c(void);
void d(void);

#endif
