/*
 * bat_testing.h
 *
 * Created: 10/26/2017 3:26:50 PM
 *  Author: Ryan Izant
 */ 


#ifndef BAT_TESTING_H_
#define BAT_TESTING_H_

#include "../global.h"

typedef struct{
	bool l2_st, l1_st, spf_st, l1_chgn, l1_faultn, l2_chgn, l2_faultn, lf_b1_bt, lf_b1_tt, lf_b2_bt, lf_b2_tt, lf_b2_chgn, lf_b2_faultn, lf_b1_chgn, lf_b1_faultn;
	float l1_sns, l2_sns, lion_ref, sp_out_ref;
	float LFB1OSNS, LFB1SNS, LFB2OSNS, LFB2SNS, LF1REF, LF2REF, LF3REF, LF4REF, L1_REF, L2_REF;
	bool LF_B2_OUTEN, LF_B1_OUTEN, L1_RUN_CHG, L2_RUN_CHG, LF_B1_RUNCHG, LF_B2_RUNCHG, L1_DISG, L2_DISG;
} bat_testing_struct;

void bat_testing_init(bat_testing_struct *data);
void readRemoteADC_0(float* batReadings);
//uint16_t readFromADC(enum adc_positive_input pin, int num_avg);
void readCommandAndSend(float* remoteBatReadings, float* batReadings, uint16_t* gpio_rs, bat_testing_struct *data);
void bat_testing_run(void);
void resetState(void);
bool areVoltagesNominal(bat_testing_struct data);
bool isLFPOOK(float bat);
bool isLionOK(float bat);

#define maxLion 4250.	// mV
#define minLion 3200.	// mV
#define maxLFPO 3650.	// mV
#define minLFPO 2500.	// mV
#define maxLionSns 3000.// mA
#define maxLFPOSns 40. 	// A


#define LEN_BAT_ADC 10

#endif /* BAT_TESTING_H_ */