/*
 * rtos_tasks.c
 *
 * Created: 9/27/2016 8:21:58 PM
 *  Author: rj16
 */ 

#include "rtos_tasks.h"
#include "processor_drivers\USART_Commands.h"

void add_lion_volts_batch_if_ready(lion_volts_batch *batch_list, int *data_array_tails, int *reads_since_last_log, int reads_per_log);
void add_lion_current_batch_if_ready(lion_current_batch *batch_list, int *data_array_tails, int *reads_since_last_log, int reads_per_log);
void add_led_temps_batch_if_ready(led_temps_batch *batch_list, int *data_array_tails, int *reads_since_last_log, int reads_per_log);
void add_lifepo_current_batch_if_ready(lifepo_current_batch *batch_list, int *data_array_tails, int *reads_since_last_log, int reads_per_log);

void increment_data_type(uint16_t data_type, int *data_array_tails, int *loops_since_last_log)
{
	// increment array tail marker and reset reads-per-log counter
	data_array_tails[data_type] = data_array_tails[data_type] + 1;
	loops_since_last_log[data_type] = 0;
}

/* Action Tasks */
static int num_tries = 0;
void antenna_deploy_task(void *pvParameters) {
	TickType_t xNextWakeTime = xTaskGetTickCount();

	for( ;; )
	{
		vTaskDelayUntil(&xNextWakeTime, ANTENNA_DEPLOY_TASK_FREQ / portTICK_PERIOD_MS);
		// set DET_RTN (antenna deployment pin) as input
		setup_pin(false, P_DET_RTN);
		// if it's open kill the task because the antenna has been deployed
		// or kill it if it's run more than 5 times because it's a lost cause
		if (get_input(P_DET_RTN) || num_tries >= 5) {
			vTaskDelete(NULL);
			} else {
			int mod_tries = num_tries % 3;
			if (mod_tries == 0 && true /* LiON is sufficiently charged and enough time has passed*/) {
				num_tries++;
				configure_pwm(P_ANT_DRV1, P_ANT_DRV1_MUX);
				set_pulse_width_fraction(3, 4);
				} else if (mod_tries != 0 && true /*LiFePO is sufficiently charged and enough time has passed*/){
				num_tries++;
				if (mod_tries == 1) {
					configure_pwm(P_ANT_DRV2, P_ANT_DRV2_MUX);
					set_pulse_width_fraction(3, 4);
					} else {
					configure_pwm(P_ANT_DRV3, P_ANT_DRV3_MUX);
					set_pulse_width_fraction(3, 4);
				}
			}
		}
	}
	
	// delete this task if it ever breaks out
	vTaskDelete(NULL);
}

void battery_charging_task(void *pvParameters)
{
	// initialize xNextWakeTime onces
	TickType_t xNextWakeTime = xTaskGetTickCount();
	
	for ( ;; )
	{
		vTaskDelayUntil( &xNextWakeTime, BATTERY_CHARGING_TASK_FREQ / portTICK_PERIOD_MS);
		
		// TODO 
	}
	// delete this task if it ever breaks out
	vTaskDelete( NULL );
}

void flash_activate_task(void *pvParameters)
{
	// initialize xNextWakeTime onces
	TickType_t xNextWakeTime = xTaskGetTickCount();
	
	for ( ;; )
	{
		vTaskDelayUntil( &xNextWakeTime, FLASH_ACTIVATE_TASK_FREQ / portTICK_PERIOD_MS);
		
		taskResumeIfSuspended(flash_data_task_handle, FLASH_DATA_TASK);
		
		// TODO: actually flash leds
	
		vTaskSuspend(flash_data_task_handle);		
	}
	// delete this task if it ever breaks out
	vTaskDelete( NULL );
}

void transmit_task(void *pvParameters)
{
	// initialize xNextWakeTime once
	TickType_t xNextWakeTime = xTaskGetTickCount();

	for( ;; )
	{
		// block for a time based on this task's globally-set frequency
		// (Note: changes to the frequency can be delayed in taking effect by as much as the past frequency...)
		vTaskDelayUntil( &xNextWakeTime, TRANSMIT_TASK_FREQ / portTICK_PERIOD_MS);
		
		// start up the data collection task so that its records data while we're transmitting
		taskResumeIfSuspended(transmit_data_task_handle, TRANSMIT_DATA_TASK);

		// check that global buffer has not been broken (TODO: ERROR)
		assert(get_msg_buffer()[0] == 'K' && get_msg_buffer()[1] == '1' && get_msg_buffer()[2] == 'A' && get_msg_buffer()[3] == 'D');
		
		// TODO: Do we need this loop? Can we just assume we'll always have data??
		bool validDataTransmitted = false; // we're cynical
		do
		{
			// read the next state to transmit (get first off state queue) 
			int nextState = (int*) equistack_Get(&last_reading_type_equistack, 0); 
			
			// grab idle data and write to header
			idle_data_t* cur_data = equistack_Get(&idle_readings_equistack, 0);
			assert(MSG_HEADER_LENGTH == sizeof(idle_data_t));
			// TODO: like to have a get_msg_buffer_header() that returns the pointer to the header section of the buffer
			//memcpy(get_msg_buffer_header(), cur_data, HEADER_LENGTH); 
			
			// TOOD: Get errors and add them (would also like to get an address of the error section)
			
			// based on what state we're in, compile a different message
			switch(nextState)
			{
				case ATTITUDE_DATA: ; // empty statement to allow definition
					// TODO: like to have a get_msg_buffer_data() that returns the pointer to the data section of the buffer
					attitude_data_t* attitude_data_trans = NULL; // (attitude_data_t*) get_msg_buffer_data();
					
					assert(ATTITUDE_DATA_PACKETS <= ATTITUDE_STACK_MAX);
					for (uint8_t i = 0; i < ATTITUDE_DATA_PACKETS; i++) {
						// TODO: What's with first arg?
						memcpy(&(attitude_data_trans[i]), equistack_Get(&attitude_readings_equistack, 0), sizeof(attitude_data_t));
					}
					
					//if (attitude_data_trans != NULL) { validDataTransmitted = true; } // TODO: If we got any invalid data (all null?)... stop?
					break;
				case TRANSMIT_DATA: ; // empty statement to allow definition
					break;
				case FLASH_DATA: ; // empty statement to allow definition
					break;
				default:
					validDataTransmitted = true; // if the state equistack is empty, we have no data, so avoid looping until we get some (potentially infinitely)
			};
		} while (!validDataTransmitted);
		
		vTaskSuspend(transmit_data_task_handle);
		
	}
	// delete this task if it ever breaks out
	vTaskDelete( NULL );
}

/* Data Read Tasks */
void current_data_task(void *pvParameters)
{
	// initialize xNextWakeTime onces
	TickType_t xNextWakeTime = xTaskGetTickCount();
	
	// tracking arrays
	uint8_t loops_since_last_log[NUM_DATA_TYPES]; // TODO: what happens if one of the data read tasks never reads
												// one of the sensors, so the value in here keeps growing?
												
	// NOTE: data_array_tails should be this big for all data reading tasks 
	// (this is just done for indexing consistency so that the add_*_batch_if_ready functions are universal) 
	uint8_t data_array_tails[NUM_DATA_TYPES];
	
	// initialize first struct
	idle_data_t *current_struct = (idle_data_t*) equistack_Initial_Stage(&idle_readings_equistack);
	assert(current_struct != NULL); // TESTING
	current_struct->timestamp = get_current_timestamp();
		
	for( ;; )
	{
		// block for a time based on a frequency, determined by whether we're in IDLE or LOW_POWER mode.
		// (Note: changes to the frequency can be delayed in taking effect by as much as the past frequency...)
		if (CurrentState == LOW_POWER) {
			vTaskDelayUntil( &xNextWakeTime, CURRENT_DATA_LOW_POWER_TASK_FREQ / portTICK_PERIOD_MS);
		} else {
			vTaskDelayUntil( &xNextWakeTime, CURRENT_DATA_TASK_FREQ / portTICK_PERIOD_MS);
		}

		// once we've collected all the data we need to into the current struct, add the whole thing
		// (all data is collected once some sensor is just about to log past the end of the list -> if one is, all should be)
		if (checkIfSuspendedAndUpdate(IDLE_DATA_TASK) || data_array_tails[IR_DATA] >= idle_IR_DATA_ARR_LEN)
		{
			
			// FOR TESTING
			idle_data_t* prev_cur_struct = current_struct;
			
			// validate previous stored value in stack, getting back the next staged address we can start adding to
			current_struct = (idle_data_t*) equistack_Stage(&idle_readings_equistack);
			current_struct->timestamp = get_current_timestamp();
			
			// reset data array tails so we're writing at the start // TODO: loops_since_last_log = ...; ???
			set_all(data_array_tails, NUM_DATA_TYPES, 0);
			
			// TESTING
			assert(prev_cur_struct != current_struct);
			assert(data_array_tails[0] == 0 && data_array_tails[1] == 0 && data_array_tails[2] == 0 && data_array_tails[3] == 0 && data_array_tails[4] == 0 && data_array_tails[NUM_DATA_TYPES-1] == 0);
			assert((idle_data_t*) equistack_Get(&idle_readings_equistack, 0) == prev_cur_struct);
		}
		
		
		// TODO: DO CHECKS FOR ERRORS (TO GENERATE ERRORS) HERE
		
		
		// see if each sensor is ready to add a batch, and do so if we need to
		/**
		 * Helpful regex: (\w*)_DATA, -> 
		 * if (loops_since_last_log[$1_DATA] >= idle_$1_LOOPS_PER_LOG) { \n current_struct->$1_data[data_array_tails[$1_DATA]] = read_$1_batch(); \n increment_data_type($1_DATA, data_array_tails, loops_since_last_log);\n}
		 */
		if (loops_since_last_log[LION_VOLTS_DATA] >= idle_LION_VOLTS_LOOPS_PER_LOG) { 
		 current_struct->lion_volts_data[data_array_tails[LION_VOLTS_DATA]] = read_lion_volts_batch(); 
		 increment_data_type(LION_VOLTS_DATA, data_array_tails, loops_since_last_log);
		}
		if (loops_since_last_log[LION_CURRENT_DATA] >= idle_LION_CURRENT_LOOPS_PER_LOG) { 
		 current_struct->lion_current_data[data_array_tails[LION_CURRENT_DATA]] = read_lion_current_batch(); 
		 increment_data_type(LION_CURRENT_DATA, data_array_tails, loops_since_last_log);
		}
		if (loops_since_last_log[LED_TEMPS_DATA] >= idle_LED_TEMPS_LOOPS_PER_LOG) { 
		 current_struct->led_temps_data[data_array_tails[LED_TEMPS_DATA]] = read_led_temps_batch(); 
		 increment_data_type(LED_TEMPS_DATA, data_array_tails, loops_since_last_log);
		}
		if (loops_since_last_log[LIFEPO_CURRENT_DATA] >= idle_LIFEPO_CURRENT_LOOPS_PER_LOG) { 
		 current_struct->lifepo_current_data[data_array_tails[LIFEPO_CURRENT_DATA]] = read_lifepo_current_batch(); 
		 increment_data_type(LIFEPO_CURRENT_DATA, data_array_tails, loops_since_last_log);
		}
		if (loops_since_last_log[IR_DATA] >= idle_IR_LOOPS_PER_LOG) { 
		 current_struct->ir_data[data_array_tails[IR_DATA]] = read_ir_batch(); 
		 increment_data_type(IR_DATA, data_array_tails, loops_since_last_log);
		}
		if (loops_since_last_log[DIODE_DATA] >= idle_DIODE_LOOPS_PER_LOG) { 
		 current_struct->diode_data[data_array_tails[DIODE_DATA]] = read_diode_batch(); 
		 increment_data_type(DIODE_DATA, data_array_tails, loops_since_last_log);
		}
		if (loops_since_last_log[BAT_TEMP_DATA] >= idle_BAT_TEMP_LOOPS_PER_LOG) { 
		 current_struct->bat_temp_data[data_array_tails[BAT_TEMP_DATA]] = read_bat_temp_batch(); 
		 increment_data_type(BAT_TEMP_DATA, data_array_tails, loops_since_last_log);
		}
		if (loops_since_last_log[IR_TEMPS_DATA] >= idle_IR_TEMPS_LOOPS_PER_LOG) { 
		 current_struct->ir_temps_data[data_array_tails[IR_TEMPS_DATA]] = read_ir_temps_batch(); 
		 increment_data_type(IR_TEMPS_DATA, data_array_tails, loops_since_last_log);
		}
		if (loops_since_last_log[RADIO_TEMP_DATA] >= idle_RADIO_TEMP_LOOPS_PER_LOG) { 
		 current_struct->radio_temp_data[data_array_tails[RADIO_TEMP_DATA]] = read_radio_temp_batch(); 
		 increment_data_type(RADIO_TEMP_DATA, data_array_tails, loops_since_last_log);
		}
		if (loops_since_last_log[IMU_DATA] >= idle_IMU_LOOPS_PER_LOG) { 
		 current_struct->imu_data[data_array_tails[IMU_DATA]] = read_imu_batch(); 
		 increment_data_type(IMU_DATA, data_array_tails, loops_since_last_log);
		}
		if (loops_since_last_log[MAGNETOMETER_DATA] >= idle_MAGNETOMETER_LOOPS_PER_LOG) { 
		 current_struct->magnetometer_data[data_array_tails[MAGNETOMETER_DATA]] = read_magnetometer_batch(); 
		 increment_data_type(MAGNETOMETER_DATA, data_array_tails, loops_since_last_log);
		}
		if (loops_since_last_log[LED_CURRENT_DATA] >= idle_LED_CURRENT_LOOPS_PER_LOG) { 
		 current_struct->led_current_data[data_array_tails[LED_CURRENT_DATA]] = read_led_current_batch(); 
		 increment_data_type(LED_CURRENT_DATA, data_array_tails, loops_since_last_log);
		}
		if (loops_since_last_log[RADIO_VOLTS_DATA] >= idle_RADIO_VOLTS_LOOPS_PER_LOG) { 
		 current_struct->radio_volts_data[data_array_tails[RADIO_VOLTS_DATA]] = read_radio_volts_batch(); 
		 increment_data_type(RADIO_VOLTS_DATA, data_array_tails, loops_since_last_log);
		}
		if (loops_since_last_log[BAT_CHARGE_VOLTS_DATA] >= idle_BAT_CHARGE_VOLTS_LOOPS_PER_LOG) { 
		 current_struct->bat_charge_volts_data[data_array_tails[BAT_CHARGE_VOLTS_DATA]] = read_bat_charge_volts_batch(); 
		 increment_data_type(BAT_CHARGE_VOLTS_DATA, data_array_tails, loops_since_last_log);
		}
		if (loops_since_last_log[BAT_CHARGE_DIG_SIGS_DATA] >= idle_BAT_CHARGE_DIG_SIGS_LOOPS_PER_LOG) { 
		 current_struct->bat_charge_dig_sigs_data[data_array_tails[BAT_CHARGE_DIG_SIGS_DATA]] = read_bat_charge_dig_sigs_batch(); 
		 increment_data_type(BAT_CHARGE_DIG_SIGS_DATA, data_array_tails, loops_since_last_log);
		}
		if (loops_since_last_log[DIGITAL_OUT_DATA] >= idle_DIGITAL_OUT_LOOPS_PER_LOG) { 
		 current_struct->digital_out_data[data_array_tails[DIGITAL_OUT_DATA]] = read_digital_out_batch(); 
		 increment_data_type(DIGITAL_OUT_DATA, data_array_tails, loops_since_last_log);
		}
		if (loops_since_last_log[IR_DATA] >= idle_IR_LOOPS_PER_LOG) {
			current_struct->ir_data[data_array_tails[IR_DATA]] = read_ir_batch();
			increment_data_type(IR_DATA, data_array_tails, loops_since_last_log);
		}
		
		// FOR TESTING
		uint8_t ir_reads_since = loops_since_last_log[0];
		uint8_t last_sens_reads_since = loops_since_last_log[NUM_DATA_TYPES-1];
		
		// increment reads in loops_since_last_log
		increment_all(loops_since_last_log, NUM_DATA_TYPES);		
		
		// TESTING
		assert(loops_since_last_log[0] == ir_reads_since + 1);
		assert(loops_since_last_log[NUM_DATA_TYPES-1] == last_sens_reads_since + 1);
	}
	// delete this task if it ever breaks out
	vTaskDelete( NULL );
}

void transmit_data_task(void *pvParameters)
{
	// initialize xNextWakeTime onces
	TickType_t xNextWakeTime = xTaskGetTickCount();
	
	// see current_data_task for extensive comments / testing
	
	// tracking arrays
	uint8_t loops_since_last_log[NUM_DATA_TYPES];
	uint8_t data_array_tails[NUM_DATA_TYPES];
	
	transmit_data_t *current_struct = (transmit_data_t*) equistack_Initial_Stage(&flash_readings_equistack);
	
	for ( ;; )
	{
		vTaskDelayUntil( &xNextWakeTime, TRANSMIT_DATA_TASK_FREQ / portTICK_PERIOD_MS);
		
		// update current_struct if necessary
		if (checkIfSuspendedAndUpdate(TRANSMIT_DATA_TASK) || data_array_tails[LION_CURRENT_DATA] >= transmit_LION_VOLTS_DATA_ARR_LEN)
		{
			// validate previous stored value in stack, getting back the next staged address we can start adding to
			current_struct = (transmit_data_t*) equistack_Stage(&flash_readings_equistack);
			current_struct->timestamp = get_current_timestamp();
			
			// log state read
			msg_data_type_t* next_state_ptr = equistack_Stage(&last_reading_type_equistack);
			*next_state_ptr = TRANSMIT_DATA;
			
			// reset data array tails so we're writing at the start // TODO: loops_since_last_log = ...; ???
			set_all(data_array_tails, NUM_DATA_TYPES, 0);
		}
		
		// TODO: DO CHECKS FOR ERRORS (TO GENERATE ERRORS) HERE
		
		
		// see if each sensor is ready to add a batch, and do so if we need to
		if (loops_since_last_log[RADIO_TEMP_DATA] >= transmit_RADIO_TEMP_LOOPS_PER_LOG) {
			current_struct->radio_temp_data[data_array_tails[RADIO_TEMP_DATA]] = read_radio_temp_batch();
			increment_data_type(RADIO_TEMP_DATA, data_array_tails, loops_since_last_log);
		}
		if (loops_since_last_log[LION_VOLTS_DATA] >= transmit_LION_VOLTS_LOOPS_PER_LOG) {
			current_struct->lion_volts_data[data_array_tails[LION_VOLTS_DATA]] = read_lion_volts_batch();
			increment_data_type(LION_VOLTS_DATA, data_array_tails, loops_since_last_log);
		}
		if (loops_since_last_log[LION_CURRENT_DATA] >= transmit_LION_CURRENT_LOOPS_PER_LOG) {
			current_struct->lion_current_data[data_array_tails[LION_CURRENT_DATA]] = read_lion_current_batch();
			increment_data_type(LION_CURRENT_DATA, data_array_tails, loops_since_last_log);
		}
	}
	// delete this task if it ever breaks out
	vTaskDelete( NULL );
}

void flash_data_task(void *pvParameters)
{
	// initialize xNextWakeTime onces
	TickType_t xNextWakeTime = xTaskGetTickCount();
	
	// current_data_task for extensive comments / testing
	
	// tracking arrays
	uint8_t loops_since_last_log[NUM_DATA_TYPES];
	uint8_t data_array_tails[NUM_DATA_TYPES];
	
	flash_data_t *current_struct = (flash_data_t*) equistack_Initial_Stage(&attitude_readings_equistack);
	
	for ( ;; )
	{
		vTaskDelayUntil( &xNextWakeTime, FLASH_DATA_TASK_FREQ / portTICK_PERIOD_MS);
		
		// update current_struct if necessary
		if (checkIfSuspendedAndUpdate(FLASH_DATA_TASK) || data_array_tails[LED_TEMPS_DATA] >= flash_LED_TEMPS_DATA_ARR_LEN)
		{
			// validate previous stored value in stack, getting back the next staged address we can start adding to
			current_struct = (flash_data_t*) equistack_Stage(&attitude_readings_equistack);
			current_struct->timestamp = get_current_timestamp();
			
			// log state read
			msg_data_type_t* next_state_ptr = equistack_Stage(&last_reading_type_equistack);
			*next_state_ptr = FLASH_DATA;
			
			// reset data array tails so we're writing at the start // TODO: loops_since_last_log = ...; ???
			set_all(data_array_tails, NUM_DATA_TYPES, 0);
		}
		
		// TODO: DO CHECKS FOR ERRORS (TO GENERATE ERRORS) HERE
		
		
// 		// see if each sensor is ready to add a batch, and do so if we need to
		if (loops_since_last_log[LED_TEMPS_DATA] >= flash_LED_TEMPS_LOOPS_PER_LOG) {
			current_struct->led_temps_data[data_array_tails[LED_TEMPS_DATA]] = read_led_temps_batch();
			increment_data_type(LED_TEMPS_DATA, data_array_tails, loops_since_last_log);
		}
		if (loops_since_last_log[LIFEPO_VOLTS_DATA] >= flash_LED_TEMPS_LOOPS_PER_LOG) {
			current_struct->lifepo_current_data[data_array_tails[LIFEPO_VOLTS_DATA]] = read_lifepo_current_batch();
			increment_data_type(LIFEPO_VOLTS_DATA, data_array_tails, loops_since_last_log);
		}
		if (loops_since_last_log[LIFEPO_CURRENT_DATA] >= flash_LIFEPO_CURRENT_LOOPS_PER_LOG) {
			current_struct->lifepo_volts_data[data_array_tails[LIFEPO_CURRENT_DATA]] = read_lifepo_volts_batch();
			increment_data_type(LIFEPO_CURRENT_DATA, data_array_tails, loops_since_last_log);
		}
		if (loops_since_last_log[LED_CURRENT_DATA] >= idle_LED_CURRENT_LOOPS_PER_LOG) {
			current_struct->led_current_data[data_array_tails[LED_CURRENT_DATA]] = read_led_current_batch();
			increment_data_type(LED_CURRENT_DATA, data_array_tails, loops_since_last_log);
		}
	}
	// delete this task if it ever breaks out
	vTaskDelete( NULL );
}

void attitude_data_task(void *pvParameters)
{
	// initialize xNextWakeTime onces
	TickType_t xNextWakeTime = xTaskGetTickCount();
	
	// current_data_task for extensive comments / testing
		
	// tracking arrays
	uint8_t loops_since_last_log[NUM_DATA_TYPES];
	uint8_t data_array_tails[NUM_DATA_TYPES];
	
	attitude_data_t *current_struct = (attitude_data_t*) equistack_Initial_Stage(&attitude_readings_equistack);
	
	for ( ;; )
	{
		vTaskDelayUntil( &xNextWakeTime, ATTITUDE_DATA_TASK_FREQ / portTICK_PERIOD_MS);
		
		// update current_struct if necessary
		if (checkIfSuspendedAndUpdate(ATTITUDE_DATA_TASK) || data_array_tails[IR_DATA] >= attitude_IR_DATA_ARR_LEN)
		{
			// validate previous stored value in stack, getting back the next staged address we can start adding to
			current_struct = (attitude_data_t*) equistack_Stage(&attitude_readings_equistack);
			current_struct->timestamp = get_current_timestamp();
			
			// log state read
			msg_data_type_t* next_state_ptr = equistack_Stage(&last_reading_type_equistack);
			*next_state_ptr = ATTITUDE_DATA;
			
			// reset data array tails so we're writing at the start // TODO: loops_since_last_log = ...; ???
			set_all(data_array_tails, NUM_DATA_TYPES, 0);
		}
		
		// TODO: DO CHECKS FOR ERRORS (TO GENERATE ERRORS) HERE
		
		
		// see if each sensor is ready to add a batch, and do so if we need to
		if (loops_since_last_log[IR_DATA] >= attitude_IR_LOOPS_PER_LOG) {
			current_struct->ir_data[data_array_tails[IR_DATA]] = read_ir_batch();
			increment_data_type(IR_DATA, data_array_tails, loops_since_last_log);
		}
		if (loops_since_last_log[DIODE_DATA] >= attitude_DIODE_LOOPS_PER_LOG) {
			current_struct->diode_data[data_array_tails[DIODE_DATA]] = read_diode_batch();
			increment_data_type(DIODE_DATA, data_array_tails, loops_since_last_log);
		}
		if (loops_since_last_log[IMU_DATA] >= attitude_IMU_LOOPS_PER_LOG) {
			current_struct->imu_data[data_array_tails[IMU_DATA]] = read_imu_batch();
			increment_data_type(IMU_DATA, data_array_tails, loops_since_last_log);
		}
		if (loops_since_last_log[MAGNETOMETER_DATA] >= attitude_MAGNETOMETER_LOOPS_PER_LOG) {
			current_struct->magnetometer_data[data_array_tails[MAGNETOMETER_DATA]] = read_magnetometer_batch();
			increment_data_type(MAGNETOMETER_DATA, data_array_tails, loops_since_last_log);
		}
	}
	// delete this task if it ever breaks out
	vTaskDelete( NULL );
}

/************************************************************************/
/* Helper Functions														*/
/************************************************************************/
void taskResumeIfSuspended(TaskHandle_t task_handle, task_type_t taskId) 
{
	if (task_handle != NULL && eTaskGetState(task_handle) == eSuspended) 
	{
		vTaskResume(task_handle); // actually resume task
		TaskSuspendedStates |= (1 << taskId); // note we WERE suspended
	}
}

/* Checks and returns whether this task was suspended, AND report that it is not suspended */
bool checkIfSuspendedAndUpdate(task_type_t taskId) {
	bool val = TaskSuspendedStates & (1 << taskId); // check the state (>0 if was suspended)
	TaskSuspendedStates &= ~(1 << taskId); // set our suspended bit to 0
	return val;
}

uint32_t get_current_timestamp()
{
	// TODO: get a more accurate and persistent timestamp (relative to an alive message)
	return xTaskGetTickCount(); // represents the ms from vstartscheduler
}

void increment_all(uint8_t* int_arr, uint8_t length)
{
	for(uint8_t i = 0; i < length; i++)
	{
		int_arr[i] = int_arr[i] + 1;
	}
}

void set_all(uint8_t* int_arr, uint8_t length, int value)
{
	for(uint8_t i = 0; i < length; i++)
	{
		int_arr[i] = value;
	}
}