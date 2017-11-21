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


void bat_testing_init(bat_testing_struct *data){

	delay_init();
	init_rtc();
	configure_i2c_standard(SERCOM4);


	// Initialize all booleans to false.
	data->LF_B2_OUTEN	= false;
	data->LF_B1_OUTEN	= false;
	data->L1_RUN_CHG	= false;
	data->L2_RUN_CHG	= false;
	data->LF_B1_RUNCHG	= false;
	data->LF_B2_RUNCHG	= false;
	data->L1_DISG  		= false;
	data->L2_DISG 		= false;


	//Initialize Digital Outputs
	setup_pin(true,P_5V_EN);
	set_output(true, P_5V_EN); //init gpio pin for 5V regulator enable
	
	setup_pin(true, P_IR_PWR_CMD);
	set_output(true,P_IR_PWR_CMD);

	// Should already be set up and made false
	set_output(data->LF_B2_OUTEN,P_LF_B2_OUTEN);
	set_output(data->LF_B1_OUTEN,P_LF_B1_OUTEN);

	setup_pin(true,P_L1_RUN_CHG);
	set_output(data->L1_RUN_CHG,P_L1_RUN_CHG);

	setup_pin(true,P_L2_RUN_CHG);
	set_output(data->L2_RUN_CHG,P_L2_RUN_CHG);

	setup_pin(true,P_LF_B1_RUNCHG);
	set_output(data->LF_B1_RUNCHG,P_LF_B1_RUNCHG);

	setup_pin(true,P_LF_B2_RUNCHG);
	set_output(data->LF_B2_RUNCHG,P_LF_B2_RUNCHG);

	setup_pin(true,P_L1_DISG);
	set_output(data->L1_DISG,P_L1_DISG);

	setup_pin(true,P_L2_DISG);
	set_output(data->L2_DISG,P_L2_DISG);
	
	LTC1380_init();
	AD7991_init(AD7991_BATBRD);
	AD7991_init(AD7991_CTRLBRD);
	uint16_t tca9535_rs;
	TCA9535_init(&tca9535_rs);
	if (!(tca9535_rs && 0xf4f0)){
		//TODO: some sort of check here. As long as we're not charging or balancing (and no batteries or solar panels are connected), f4f0 should be the return value.
	} else{
		uint16_t res = tca9535_rs;
		data->l2_st 		= (res>>8)&0x1;	//8
		data->l1_st 		= (res>>9)&0x1;//9
		data->spf_st 		= (res>>10)&0x1; //10
		data->l1_chgn 		= (res>>12)&0x1; //12
		data->l1_faultn 	= (res>>13)&0x1; //13
		data->l2_chgn 		= (res>>14)&0x1; //14
		data->l2_faultn 	= (res>>15)&0x1; //15
		data->lf_b1_bt 		= res&0x1;//0
		data->lf_b1_tt 		= (res>>1)&0x1; //1
		data->lf_b2_tt 		= (res>>2)&0x1; //2
		data->lf_b2_bt 		= (res>>3)&0x1; //3
		data->lf_b2_chgn 	= (res>>4)&0x1; //4
		data->lf_b2_faultn 	= (res>>5)&0x1; //5
		data->lf_b1_chgn 	= (res>>6)&0x1; //6
		data->lf_b1_faultn 	= (res>>7)&0x1; //7
	}
	
}

void resetState(void) {
	set_output(false, P_LF_B2_OUTEN);
	set_output(false, P_LF_B1_OUTEN);
	set_output(false, P_L1_RUN_CHG);
	set_output(false, P_L2_RUN_CHG);
	set_output(false, P_LF_B1_RUNCHG);
	set_output(false, P_LF_B2_RUNCHG);
	set_output(false, P_L1_DISG);
	set_output(false, P_L2_DISG);

	resetBatOutputs();
}

//Reads all sensors from Battery Board remote ADC
void readRemoteADC_0(float* readings){
	uint16_t remoteADC[4];
	AD7991_read_all(remoteADC, AD7991_BATBRD);
	readings[0] = ((float) remoteADC[0]);
	readings[1] = ((float) remoteADC[1]);
	readings[2]= ((float)  remoteADC[2]);
	readings[3] = ((float) remoteADC[3]);
}

static void readBatBoard(float* batBoardReadings){
	uint16_t raw[LEN_BAT_ADC];
	for (int i=0; i<LEN_BAT_ADC; i++){
		uint16_t buf;
		readFromADC(bat_adc_pins[i],10,&buf);
		raw[i] = buf;
		batBoardReadings[i] = convertToVoltage(raw[i]);
	}
	
}

// Given an ADC channel, reads from ADC with <num_avg> software averaging
/*uint16_t readFromADC(enum adc_positive_input pin, int num_avg){
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
}*/

void readCommandAndSend(float* remoteBatReadings, float* batReadings, uint16_t* gpio_rs, bat_testing_struct *data){

	
	readRemoteADC_0(remoteBatReadings);
	data->l1_sns 		= (((float) remoteBatReadings[1])/4096*3.3-0.985)*2000;	// mA
	data->l2_sns 		= (((float) remoteBatReadings[0])/4096*3.3-1.022)*2000;	// mA
	data->lion_ref 		= ((float)  (remoteBatReadings[2])/4096*3.3-0.05)*2717;	// mV
	data->sp_out_ref	= ((float) (remoteBatReadings[3])/4096*3.3-0.13)*5580;	// mV

	
	readBatBoard(batReadings);
	//Testing will have to determine if we're ok to multiply by 1000 to convert to mA
	data->LFB1OSNS 	= batReadings[0]*71;			// A 
	data->LFB1SNS 	= (batReadings[1]*-0.980)*50;	// A
	data->LFB2OSNS 	= batReadings[2]*71.43;			// A 
	data->LFB2SNS 	= (batReadings[3]-0.979)*50;	// A 
	
	data->LF2REF = batReadings[5]*1950; 				// mV
	data->LF1REF = (batReadings[4]*3870)-data->LF2REF; 		// mV

	data->LF4REF = batReadings[7]*1950; 				// mV
	data->LF3REF = (batReadings[6]*3870)-data->LF4REF; 	// mV

	data->L1_REF = batReadings[8]*2500; 				// mV
	data->L2_REF = batReadings[9]*2500; 				// mV
	
	
	uint8_t setOutputs[] = {data->lf_b2_bt,data->lf_b2_tt,data->lf_b1_tt,data->lf_b1_bt};
	setBatOutputs(setOutputs[0]);
	readTCA9535Levels(gpio_rs);
	uint16_t res = gpio_rs;
	data->l2_st 		= (res>>8)&0x1;		//8
	data->l1_st 		= (res>>9)&0x1;		//9
	data->spf_st 		= (res>>10)&0x1;	//10
	data->l1_chgn 		= (res>>12)&0x1;	//12
	data->l1_faultn 	= (res>>13)&0x1;	//13
	data->l2_chgn 		= (res>>14)&0x1;	//14
	data->l2_faultn 	= (res>>15)&0x1;	//15
	data->lf_b2_chgn 	= (res>>4)&0x1;		//4
	data->lf_b2_faultn 	= (res>>5)&0x1;		//5
	data->lf_b1_chgn 	= (res>>6)&0x1;		//6
	data->lf_b1_faultn 	= (res>>7)&0x1;		//7
	// could also grab the outputs, indices 0,1,2,3 and verify that they match what we chose the output to be at that moment

	
	
	
	print("%d;%d;%d;%d;%d;%d;%d;%d;%d;",get_rtc_count(), (int) data->LF_B2_OUTEN, (int) data->LF_B1_OUTEN, (int) data->L1_RUN_CHG, (int) data->L2_RUN_CHG, (int) data->LF_B1_RUNCHG, (int) data->LF_B2_RUNCHG, (int) data->L1_DISG, (int) data->L2_DISG);
	print("%d;%d;%d;%d;", (int) data->l1_sns, (int) data->l2_sns, (int) data->lion_ref, (int) data->sp_out_ref);
	print("%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;", (int) data->LFB1OSNS, (int) data->LFB1SNS, (int) data->LFB2OSNS, (int) data->LFB2SNS, (int) data->LF1REF, (int) data->LF2REF, (int) data->LF3REF, (int) data->LF4REF, (int) data->L1_REF, (int) data->L2_REF);
	print("%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d\n", (int) data->l2_st, (int) data->l1_st, (int) data->spf_st, (int) data->l1_chgn, (int) data->l1_faultn, (int) data->l2_chgn, (int) data->l2_faultn, (int) data->lf_b1_bt, (int) data->lf_b1_tt, (int) data->lf_b2_bt, (int) data->lf_b2_tt, (int) data->lf_b2_chgn, (int) data->lf_b2_faultn, (int) data->lf_b1_chgn, (int) data->lf_b1_faultn);
}

bool areVoltagesNominal(bat_testing_struct data){
	float l1 = data.L1_REF;
	float l2 = data.L2_REF;
	float lf1 = data.LF1REF;
	float lf2 = data.LF2REF;
	float lf3 = data.LF3REF;
	float lf4 = data.LF4REF;
	return !(!isLionOK(l1) | !isLionOK(l2) | !isLFPOOK(lf1) | !isLFPOOK(lf2) | !isLFPOOK(lf3) | !isLFPOOK(lf4));
}

bool isLionOK(float bat){
	return (bat<maxLion && bat>minLion);
}

bool isLFPOOK(float bat){
	return (bat<maxLFPO && bat>minLFPO);
}

void bat_testing_run(){
	
	float remoteBatReadings[4];
	float batReadings[LEN_BAT_ADC];
	uint16_t gpio_rs;

	bat_testing_struct bat_test_data;
	
	bat_test_data.lf_b1_bt = false;
	bat_test_data.lf_b1_tt = false;
	bat_test_data.lf_b2_tt = false;
	bat_test_data.lf_b2_tt = false;

	bat_testing_init(&bat_test_data);
	
	
	
	
	print("time[s];LF_B2_OUTEN[cmd];LF_B1_OUTEN[cmd];L1_RUN_CHG[cmd];L2_RUN_CHG[cmd];LF_B1_RUNCHG[cmd];LF_B2_RUNCHG[cmd];L1_DISG[cmd];L2_DISG[cmd];");
	print("l1_sns[mA];l2_sns[mA];lion_ref[mv];sp_out_ref[mV];");
	print("LFB1OSNS[A];LFB1SNS[A];LFB2OSNS[A];LFB2SNS[A];LF1REF[mV];LF2REF[mV];LF3REF[mV];LF4REF[mV];L1_REF[mV];L2_REF[mV];");
	print("l2_st[bool];l1_st[bool];spf_st[bool];l1_chgn[bool];l1_faultn[bool];l2_chgn[bool];l2_faultn[bool];lf_b1_bt[bool];lf_b1_tt[bool];lf_b2_bt[bool];lf_b2_tt[bool];lf_b2_chgn[bool];lf_b2_faultn[bool];lf_b1_chgn[bool];lf_b1_faultn[bool]\n");
	
	int count = 0;
	while (count<3600){
		delay_ms(961); //calibrated delay
		readCommandAndSend(remoteBatReadings, batReadings, &gpio_rs, &bat_test_data);
		
		/*if (!areVoltagesNominal(bat_test_data)){
			print("A voltage has been detected as being an anomaly. Automatically resetting outputs and exiting.\n");
			resetState();
		}*/
		if(bat_test_data.l1_sns>250 || bat_test_data.L1_REF>4400){
			set_output(false,P_L1_RUN_CHG);
		}
		if(count==30){
			set_output(true,P_L1_RUN_CHG);
		}else if (count ==3000){
			set_output(false,P_L1_RUN_CHG);
		}
		
		count = count +1;
	}
}


