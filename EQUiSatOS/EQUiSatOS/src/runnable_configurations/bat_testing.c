/*
 * bat_testing.c
 *
 * Created: 10/26/2017 3:26:26 PM
 *  Author: Ryan Izant
 */ 
#include "bat_testing.h"


enum adc_positive_input bat_adc_pins[LEN_BAT_ADC] = {
	P_AI_LFB1OSNS,
	P_AI_LFB1SNS,
	P_AI_LFB2OSNS,
	P_AI_LFB2SNS,
	P_AI_LF1REF,
	P_AI_LF2REF,
	P_AI_LF3REF,
	P_AI_LF4REF,
	P_AI_L1_REF,
	P_AI_L2_REF,
};

void bat_testing_init(){
	delay_init();
	configure_i2c_standard(SERCOM4);
	
	LTC1380_init();
	AD7991_init();
	return_struct_16 tca9535_rs;
	TCA9535_init(&tca9535_rs);
	if (!(tca9535_rs.return_value && 0xf4f0)){
		bool ohno = true; //Todo some sort of check here. As long as we're not charging, f4f0 should be the return value.
	}
	
	setup_pin(true,P_5V_EN);
	set_output(true, P_5V_EN); //init gpio pin for 5V regulator enable
	
	setup_pin(true, P_IR_PWR_CMD);
	
}

//Reads all sensors from Battery Board remote ADC
void readRemoteADC_0(float* readings){
	uint16_t remoteADC[4];
	AD7991_read_all(remoteADC, AD7991_ADDR_0);
	readings[0] = ((float) remoteADC[0]);
	readings[1] = ((float) remoteADC[1]);
	readings[2]= ((float)  remoteADC[2]);
	readings[3] = ((float) remoteADC[3]);
}

void readBatBoard(float* batBoardReadings){
	uint16_t raw[LEN_BAT_ADC];
	float rawV[LEN_BAT_ADC];
	for (int i=0; i<LEN_BAT_ADC; i++){
		raw[i] = readFromADC(bat_adc_pins[i],5);
		rawV[i] = convertToVoltage(raw);
	}
	batBoardReadings[0] = rawV[0]*71.43;		//[A] P_AI_LFB1OSNS
	batBoardReadings[1] = (rawV[1]*-0.980)*50;	//[V] P_AI_LFB1SNS
	batBoardReadings[2] = rawV[2]*71.43;		//[V] P_AI_LFB2OSNS
	batBoardReadings[3] = (rawV[3]-0.979);		//[V] P_AI_LFB2SNS
	
	batBoardReadings[5] = rawV[5]*1.95; //[V] P_AI_LF2REF
	batBoardReadings[4] = (rawV[4]*3.87)-batBoardReadings[5]; //[V] P_AI_LF1REF
	
	batBoardReadings[7] = rawV[7]*1.95; //[V] P_AI_LF4REF
	batBoardReadings[6] = (rawV[6]*3.87)-batBoardReadings[7]; //[V] P_AI_LF3REF
	
	batBoardReadings[8] = rawV[8]*2.5; //[V] P_AI_L1_REF
	batBoardReadings[9] = rawV[9]*2.5; //[V] P_AI_L2_REF
}

// Given an ADC channel, reads from ADC with <num_avg> software averaging
uint16_t readFromADC(enum adc_positive_input pin, int num_avg){
	struct adc_module adc_instance;
	configure_adc(&adc_instance,pin);
	//read_adc(adc_instance);
	//adc_enable(&adc_instance);
	uint16_t sum = 0;
	
	for (int i=0; i<num_avg; i++){
		adc_enable(&adc_instance);
		sum = sum +read_adc(adc_instance);
	}
	return sum/num_avg;
}



void bat_testing_run(){
	
	bat_testing_init();
	
	float remoteBatReadings[4];
	float batReadings[LEN_BAT_ADC];
	return_struct_16 gpio_rs;
	
	//io direction configs, 1=input, 0=output
	bool l2_st, l1_st, spf_st, l1_chgn, l1_faultn, l2_chgn, l2_faultn, lf_b1_bt, lf_b1,tt, lf_b2_bt, lf_b2_tt, lf_b2_chgn, lf_b2_faultn, lf_b1_chgn, lf_b1_faultn;
	float l1_sns, l2_sns, lion_ref, sp_out_ref;
	
	print("time[ms];l1_sns[mA];l2_sns[mA];lion_ref[mv];sp_out_ref[mV]\n");
	
	for (int i=0; i<100; i++){
		
		readRemoteADC_0(remoteBatReadings);
		l1_sns = (((float) remoteBatReadings[0])/4096*3.3-1)*2000;// mA
		l2_sns = (((float) remoteBatReadings[1])/4096*3.3-1)*2000;// mA
		lion_ref = ((float)  remoteBatReadings[2])/4096*3.3*2717;//mV
		sp_out_ref = ((float) remoteBatReadings[3])/4096*3.3*5580;//mV

		
		//readBatBoard(batReadings);
		
		//enum status = setIOMask()
		readTCA9535Levels(&gpio_rs);
		
		set_output(true,P_IR_PWR_CMD);
		
		print("%d;%d;%d;%d;%d\n",i*10,(int)l1_sns, (int)l2_sns, (int) lion_ref, (int)sp_out_ref);
		delay_ms(10);
	}
}