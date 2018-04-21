/*
 * sensor_read_commands.c
 *
 * Created: 11/1/2016 8:18:16 PM
 *  Author: jleiken
 */

#include "sensor_read_commands.h"

static uint8_t IR_ADDS[6] = {
	IR_POS_Y,
	IR_NEG_X,
	IR_NEG_Y,
	IR_POS_X,
	IR_NEG_Z,
	IR_POS_Z
};

static uint8_t IR_ELOCS[6] = {
	ELOC_IR_POS_Y,
	ELOC_IR_NEG_X,
	ELOC_IR_NEG_Y,
	ELOC_IR_POS_X,
	ELOC_IR_NEG_Z,
	ELOC_IR_POS_Z
};

static uint8_t TEMP_ELOCS[8] = {
	ELOC_TEMP_LF_1,
	ELOC_TEMP_LF_2,
	ELOC_TEMP_L_1,
	ELOC_TEMP_L_2,
	ELOC_TEMP_LED_1,
	ELOC_TEMP_LED_2,
	ELOC_TEMP_LED_3,
	ELOC_TEMP_LED_4
};

static uint8_t PD_ELOCS[6] = {
	ELOC_PD_POS_Y,
	ELOC_PD_NEG_X,
	ELOC_PD_NEG_Y,
	ELOC_PD_POS_X,
	ELOC_PD_NEG_Z,
	ELOC_PD_POS_Z,
};

static struct adc_module adc_instance; // global is allowed because we always lock the processor ADC

void init_sensor_read_commands(void) {
	i2c_irpow_mutex = xSemaphoreCreateMutexStatic(&_i2c_irpow_mutex_d);
	processor_adc_mutex = xSemaphoreCreateMutexStatic(&_processor_adc_mutex_d);
	irpow_semaphore = xSemaphoreCreateCountingStatic(IR_POW_SEMAPHORE_MAX_COUNT, IR_POW_SEMAPHORE_MAX_COUNT, &_irpow_semaphore_d);
}

/************************************************************************/
/* HELPERS                                                              */
/************************************************************************/
/* NOTE: the "batch" value passed into these functions are generally arrays, so are passed by reference */

// unsafe version required for verify_regulators_unsafe
static void read_ad7991_ctrlbrd_unsafe(ad7991_ctrlbrd_batch batch) {
	status_code_genare_t sc = AD7991_read_all_mV(batch, AD7991_CTRLBRD);
	log_if_error(ELOC_AD7991_CBRD, sc, false);
}

uint8_t truncate_16t(uint16_t src, sig_id_t sig) {
	uint16_t m = get_line_m_from_signal(sig);
	int16_t b = get_line_b_from_signal(sig);
	return (((uint16_t)(src + b)) * m) >> 8;
}

void log_if_out_of_bounds(uint16_t reading, sig_id_t sig, uint8_t eloc, bool priority) {
	uint16_t low = get_low_bound_from_signal(sig);
	uint16_t high = get_high_bound_from_signal(sig);
	if (reading < low) {
		log_error(eloc, ECODE_READING_LOW, priority);
		#ifdef PRINT_NEW_ERRORS
			sat_error_t err;
			err.ecode = 0;
			err.timestamp = 0;
			err.eloc = eloc;
			print("READING OUT OF BOUNDS - LOW - eloc: %s, reading: %d, low bound: %d\n", get_eloc_str(&err), reading, low);
		#endif
	} else if (reading > high) {
		log_error(eloc, ECODE_READING_HIGH, priority);
		#ifdef PRINT_NEW_ERRORS
			sat_error_t err;
			err.ecode = 0;
			err.timestamp = 0;
			err.eloc = eloc;
			print("READING OUT OF BOUNDS - HIGH - eloc: %s, reading: %d, high bound: %d\n", get_eloc_str(&err), reading, high);
		#endif
	}
}

// note: processor ADC is locked externally to these methods for speed and for particular edge cases
static void commands_read_adc_mV(uint16_t* dest, uint8_t pin, uint8_t eloc, sig_id_t sig, bool priority, bool precise) {
	status_code_genare_t sc = configure_adc(&adc_instance, pin, precise);
	log_if_error(eloc, sc, priority);
	sc = read_adc_mV(adc_instance, dest);
	log_if_error(eloc, sc, priority);
	log_if_out_of_bounds(*dest, sig, eloc, priority);
}

static void commands_read_adc_mV_truncate(uint8_t* dest, int pin, uint8_t eloc, sig_id_t sig, bool priority) {
	uint16_t read;
	commands_read_adc_mV(&read, pin, eloc, sig, priority, false);
	*dest = truncate_16t(read, sig);
}

// unsafe because need i2c_mutex AND/OR processor_adc_mutex
void _set_5v_enable_unsafe(bool on) {
	// note: to avoid chance of deadlock, any locks
	// of the i2c bus / processor ADC mutex must be above this
	bool got_hw_state_mutex = hardware_state_mutex_take(ELOC_5V_REF);
	// enable regulator (taking mutex in case anyone wants consistent state),
	// but don't set the hardware state until after the delay
	set_output(on, P_5V_EN);
	get_hw_states()->rail_5v_enabled = HW_TRANSITIONING;
	if (got_hw_state_mutex) hardware_state_mutex_give();

	// allow time to power up/down before someone uses it
	if (on) {
		vTaskDelay(EN_5V_POWER_ON_DELAY_MS / portTICK_PERIOD_MS);
		
		// set hardware state after sufficient time for it to be valid
		got_hw_state_mutex = hardware_state_mutex_take(ELOC_5V_REF);
		get_hw_states()->rail_5v_enabled = on;
		if (got_hw_state_mutex)	hardware_state_mutex_give();
		
	} else {
		// don't delay on OFF time because it's so long... just let 
		// tasks know they have to wait
		// NOTE: if tick count is going to overflow, it will overflow here such AND
		// will overflow as tasks are waiting, so it will work fine (worst case someone will read a bad reading)
		get_hw_states()->rail_5v_target_off_time = xTaskGetTickCount() + (EN_5V_POWER_OFF_DELAY_MS / portTICK_PERIOD_MS);
	}
}

// wrapper function to handle enabling IR power;
// uses counting semaphore to keep track of IR power users.
// Also makes sure that this function does not return (indicating IR power ready),
// unless the wait time for it to power up has completed.
bool enable_ir_pow_if_necessary(void) {
	// increment the IR power semaphore to indicate there is another user.
	bool got_semaphore = xSemaphoreTake(irpow_semaphore, HARDWARE_MUTEX_WAIT_TIME_TICKS);
	if (!got_semaphore) {
		log_error(ELOC_IR_POW, ECODE_IRPOW_SEM_TOO_MANY_USERS, true);
	}
	
	if (!get_output(P_IR_PWR_CMD)) {
		// only enable (and do full delay) if IR power is not on
		trace_print("set ir power on");
		set_output(true, P_IR_PWR_CMD);
		// NOTE: if tick count is going to overflow, it will overflow here such AND
		// will overflow as tasks are waiting, so it will work fine (worst case someone will read a bad reading)
		get_hw_states()->ir_target_on_time = xTaskGetTickCount() + (IR_WAKE_DELAY_MS / portTICK_PERIOD_MS);
		vTaskDelay(IR_WAKE_DELAY_MS / portTICK_PERIOD_MS);

	} else {
		// if we read that someone enabled it, make sure to delay
		// in case it was enabled less than IR_WAKE_DELAY_MS ago
		// note wake_time is technically updated, so use a local
		// to avoid concurrency issues
		TickType_t wake_time = get_hw_states()->ir_target_on_time;
		vTaskDelayUntil(&wake_time, 1); // can't do delay offset of 0
	}
	return got_semaphore;
}

// wrapper function to handle disabling IR power;
// looks at count on IR power counting semaphore to determine
// whether there are more users of IR power, and disables it if there are none
// Takes in whether the first call to enable_ir_pow_if_necessary got the semaphore
// to avoid bad cases in the off chance it didn't
void disable_ir_pow_if_necessary(bool got_semaphore) {
	bool could_give = false; // if didn't get semaphore initially, that's also bad
	if (got_semaphore) {
		could_give = xSemaphoreGive(irpow_semaphore);
		if (!could_give) {
			log_error(ELOC_IR_POW, ECODE_IRPOW_SEM_TOO_FEW_USERS, true);
		}
	}
	// check if there are no IR power users, and disable if there are none
	// NOTE: if we couldn't give the IR power semaphore, that means too many people
	// took it, and someone is probably still using it, so don't turn it off in here
	// (use the i2c mutex instead at some point)
	if (could_give && uxSemaphoreGetCount(irpow_semaphore) == IR_POW_SEMAPHORE_MAX_COUNT) {
		set_output(false, P_IR_PWR_CMD);
		trace_print("set ir power off");
	}
}

// disables IR power if it's still on and the semaphore indicates no users;
// run periodically in case someone left it on somehow. Note this always
// will turn off IR power unless (rarely) it times out
void ensure_ir_power_disabled(bool expected_on) {
	// if the semaphore says that there are no users, we can definitely turn off IR power
	// otherwise, if the semaphore is not reading now users,
	// try and take the i2c_mutex. If we can obtain it, we know there
	// are no active users and we can shut off IR power
	if (uxSemaphoreGetCount(irpow_semaphore) == IR_POW_SEMAPHORE_MAX_COUNT) {
		set_output(false, P_IR_PWR_CMD);
		trace_print("set ir power off (semaphore unused)");

	} else if (xSemaphoreTake(i2c_irpow_mutex, HARDWARE_MUTEX_WAIT_TIME_TICKS)) {
		trace_print("set ir power off (had to take mutex)");
		set_output(false, P_IR_PWR_CMD);
		xSemaphoreGive(i2c_irpow_mutex);

	} else {
		// we can't turn it off if there are still tasks using it or the i2c mutex
		log_error(ELOC_IR_POW, ECODE_I2C_IRPOW_MUTEX_TIMEOUT, false);
		return;
	}

	if (!expected_on && get_output(P_IR_PWR_CMD)) {
		// if it was expected to be off and it's on, this is an issue
		log_error(ELOC_IR_POW, ECODE_INCONSISTENT_STATE, false);
		configASSERT(false); // this should never happen if mutex system is good
	}
}

void verify_regulators_unsafe(void) {
	struct hw_states* states;
	ad7991_ctrlbrd_batch batch = {0,0,0,0};

	// only lock hardware state mutex while needed to act on state,
	// but long enough to ensure the state doesn't change in the middle of checking it
	if (hardware_state_mutex_take(ELOC_VERIFY_REGS)) {
		read_ad7991_ctrlbrd_unsafe(batch);
		
		// check 5V regulator state and update hardware states if necessary
		// if it's transitioning to OFF, check that it's done and set it's
		// state to off if it is (we're the one who should do this)
		if (get_hw_states()->rail_5v_enabled == HW_TRANSITIONING 
			&& xTaskGetTickCount() >= get_hw_states()->rail_5v_target_off_time) {
			get_hw_states()->rail_5v_enabled = HW_OFF;
		}
		
		// check radio power off regulator state and update hardware states if necessary
		// if it's transitioning to IDLE, check that it's done and set it's
		// state to off if it is (we're the one who should do this)
		if (get_hw_states()->radio_state == RADIO_IDLE_TRANS_TRANSITION
			&& xTaskGetTickCount() >= get_hw_states()->radio_trans_done_target_time) {
			get_hw_states()->radio_state = RADIO_IDLE;
		}
		
		states = get_hw_states(); // take at time of state read
		hardware_state_mutex_give();
	} else {
		return;
	}

	// 3V6_REF is index 0
	log_if_out_of_bounds(batch[0], states->radio_state ? S_3V6_REF_ON : S_3V6_REF_OFF, ELOC_AD7991_CBRD_3V6_REF, true);
	// 3V6_SNS is index 1
	sig_id_t state_3v6_sns;
	switch(states->radio_state) {
		case RADIO_OFF:
			state_3v6_sns = S_3V6_SNS_OFF;
			break;
		case RADIO_IDLE:
			state_3v6_sns = S_3V6_SNS_ON;
			break;
		case RADIO_TRANSMITTING:
			state_3v6_sns = S_3V6_SNS_TRANSMIT;
			print("\n=======\nRADIO CURRENT: %d mA\n=======\n",batch[1]);
			break;
		case RADIO_OFF_IDLE_TRANSITION:
			state_3v6_sns = S_3V6_SNS_OFF_IDLE_TRANSITION;
			break;
		case RADIO_IDLE_TRANS_TRANSITION:
			state_3v6_sns = S_3V6_SNS_IDLE_TRANS_TRANSITION;
			break;
		default:
			state_3v6_sns = S_3V6_REF_OFF; // most likely
			log_error(ELOC_AD7991_CBRD_3V6_REF, ECODE_UNEXPECTED_CASE, false);
			break;
	}
	log_if_out_of_bounds(batch[1], state_3v6_sns, ELOC_AD7991_CBRD_3V6_SNS, true);
	// 5VREF is index 2
	sig_id_t state_5v_rail;
	switch (states->rail_5v_enabled) {
		case HW_OFF:
			state_5v_rail = S_5VREF_OFF;
			break;
		case HW_ON:
			state_5v_rail = S_5VREF_ON;
			break;
		case HW_TRANSITIONING:
			state_5v_rail = S_5VREF_TRANSITION;
			break;
		default:
			state_5v_rail = S_5VREF_OFF; // most likely
			log_error(ELOC_5V_REF, ECODE_UNEXPECTED_CASE, false);
			break;
	}

	log_if_out_of_bounds(batch[2], state_5v_rail, ELOC_AD7991_CBRD_5V_REF, true);
	// 3V3REF current is index 3 (should always be on)
	log_if_out_of_bounds(batch[3], S_3V3_REF, ELOC_AD7991_CBRD_3V3_REF, true);
}

void verify_regulators(void) {
	if (xSemaphoreTake(i2c_irpow_mutex, HARDWARE_MUTEX_WAIT_TIME_TICKS))
	{
		// safely turn on IR power
		bool got_semaphore = enable_ir_pow_if_necessary();
		// safely delay until 5V regulator is in a good state
		TickType_t wake_time = get_hw_states()->rail_5v_target_off_time;
		vTaskDelayUntil(&wake_time, 1); // can't do delay offset of 0
		// safely delay until radio is in a good state after transmit
		wake_time = get_hw_states()->radio_trans_done_target_time;
		vTaskDelayUntil(&wake_time, 1); // can't do delay offset of 0
		
		verify_regulators_unsafe();
		disable_ir_pow_if_necessary(got_semaphore);
		xSemaphoreGive(i2c_irpow_mutex);
	} else {
		log_error(ELOC_AD7991_CBRD_3V3_REF, ECODE_I2C_IRPOW_MUTEX_TIMEOUT, false);
	}
}

/************************************************************************/
/* SENSOR BATCH READING FUNCTIONS                                       */
/************************************************************************/

void read_ir_object_temps_batch(ir_object_temps_batch batch) {
	if (xSemaphoreTake(i2c_irpow_mutex, HARDWARE_MUTEX_WAIT_TIME_TICKS))
	{
		bool got_semaphore = enable_ir_pow_if_necessary();
		// send stop because the IR sensors need it before processing commands
		// after the line has been busy
		i2c_send_stop();
		for (int i = 0; i < 6; i ++) {
			uint16_t obj;
			status_code_genare_t sc = MLX90614_read_all_obj(IR_ADDS[i], &obj);
			log_if_error(IR_ELOCS[i], sc, false);
			log_if_out_of_bounds(obj, S_IR_OBJ, IR_ELOCS[i], false);
			batch[i] = obj;
		}
		disable_ir_pow_if_necessary(got_semaphore);

		xSemaphoreGive(i2c_irpow_mutex);
	} else {
		log_error(ELOC_IR_POS_Z, ECODE_I2C_IRPOW_MUTEX_TIMEOUT, false);
		memset(batch, 0, sizeof(ir_object_temps_batch));
	}
}

void read_ir_ambient_temps_batch(ir_ambient_temps_batch batch) {
	if (xSemaphoreTake(i2c_irpow_mutex, HARDWARE_MUTEX_WAIT_TIME_TICKS))
	{
		bool got_semaphore = enable_ir_pow_if_necessary();
		// send stop because the IR sensors need it before processing commands
		// after the line has been busy
		i2c_send_stop();
		for (int i = 0; i < 6; i++) {
			uint16_t amb;
			status_code_genare_t sc = MLX90614_read2ByteValue(IR_ADDS[i], AMBIENT, &amb);
			log_if_error(IR_ELOCS[i], sc, false);
			log_if_out_of_bounds(amb, S_IR_AMB, IR_ELOCS[i], false);
			batch[i] = truncate_16t(amb, S_IR_AMB);
		}
		disable_ir_pow_if_necessary(got_semaphore);
		xSemaphoreGive(i2c_irpow_mutex);
	} else {
		log_error(ELOC_IR_POS_Y, ECODE_I2C_IRPOW_MUTEX_TIMEOUT, false);
		memset(batch, 0, sizeof(ir_ambient_temps_batch));
	}
}

void read_lion_volts_batch(lion_volts_batch batch) {
	uint16_t val_1_precise;
	uint16_t val_2_precise;

	// locks and releases processor_adc_mutex
	read_lion_volts_precise(&val_1_precise, &val_2_precise, false);
	batch[0] = truncate_16t(val_1_precise, S_L_VOLT);
	batch[1] = truncate_16t(val_2_precise, S_L_VOLT);
}

bool read_lion_volts_precise(uint16_t* val_1, uint16_t* val_2, bool precise) {
	if (xSemaphoreTake(processor_adc_mutex, HARDWARE_MUTEX_WAIT_TIME_TICKS))
	{
		#ifndef EQUISIM_SIMULATE_BATTERIES
			commands_read_adc_mV(val_1, P_AI_L1_REF, ELOC_L1_REF, S_L_VOLT, true, precise);
			commands_read_adc_mV(val_2, P_AI_L2_REF, ELOC_L2_REF, S_L_VOLT, true, precise);
			*val_1 = *val_1 * 25 / 10;
			*val_2 = *val_2 * 25 / 10;
		#else
			equisim_read_lion_volts_precise(val_1, val_2);
		#endif

		xSemaphoreGive(processor_adc_mutex);
		return true;
	} else {
		log_error(ELOC_L1_REF, ECODE_PROC_ADC_MUTEX_TIMEOUT, false);
		memset(val_1, 0, sizeof(uint16_t));
		memset(val_2, 0, sizeof(uint16_t));
		return false;
	}
}

//results must be length 4
bool read_ad7991_batbrd_precise(uint16_t* results) {
	status_code_genare_t sc;
	sig_id_t l1_sig;
	sig_id_t l2_sig;
	sig_id_t sig;

	// (we need to lock i2c_irpow_mutex before hardware_state_mutex to avoid deadlock)
	if (xSemaphoreTake(i2c_irpow_mutex, HARDWARE_MUTEX_WAIT_TIME_TICKS)) {
		bool got_semaphore = enable_ir_pow_if_necessary();
		// only lock hardware state mutex while needed to act on state,
		// but long enough to ensure the state doesn't change in the middle of checking it
		bool got_hw_state_mutex = hardware_state_mutex_take(ELOC_AD7991_BBRD);
		{
			sc = AD7991_read_all_mV(results, AD7991_BATBRD);
			log_if_error(ELOC_AD7991_BBRD, sc, true);

			struct hw_states* states = get_hw_states();
			int8_t disg_state = charging_data.lion_discharging;
			if (states->antenna_deploying) {
				sig = S_L_SNS_ANT_DEPLOY;
			} else if (states->radio_state == RADIO_TRANSMITTING) {
				sig = S_L_SNS_TRANSMIT;
			} else if (states->radio_state == RADIO_IDLE_TRANS_TRANSITION) {
				sig = S_L_SNS_IDLE_TRANS_TRANSITION;
			} else if (states->radio_state == RADIO_OFF_IDLE_TRANSITION) {
				sig = S_L_SNS_OFF_IDLE_TRANSITION;
			} else if (states->radio_state == RADIO_IDLE) {
				sig = S_L_SNS_IDLE_RAD_ON;
			} else {
				// default; most likely
				sig = S_L_SNS_IDLE_RAD_OFF;
			}
			l1_sig = (disg_state == LI1 || disg_state == -1) ? sig : S_L_SNS_OFF;
			l2_sig = (disg_state == LI2 || disg_state == -1) ? sig : S_L_SNS_OFF;
		}
		if (got_hw_state_mutex) hardware_state_mutex_give();
		disable_ir_pow_if_necessary(got_semaphore);
		xSemaphoreGive(i2c_irpow_mutex);
	} else {
		log_error(ELOC_AD7991_BBRD, ECODE_I2C_IRPOW_MUTEX_TIMEOUT, false);
		memset(results, 0, sizeof(uint16_t)*4);
		return false;
	}

	// results[0] = L2_SNS
	log_if_out_of_bounds(results[0], l2_sig, ELOC_AD7991_BBRD_L2_SNS, true);
	// results[1] = L1_SNS
	log_if_out_of_bounds(results[1], l1_sig, ELOC_AD7991_BBRD_L1_SNS, true);
	// results[2] = L_REF
	log_if_out_of_bounds(results[2], S_LREF, ELOC_AD7991_BBRD_L_REF, true);
	// results[3] = PANELREF
	#ifdef EQUISIM_SIMULATE_BATTERIES
	results[3] = equisim_read_panelref();
	#endif
	log_if_out_of_bounds(results[3], S_PANELREF, ELOC_AD7991_BBRD_PANEL_REF, true);

	return true;
}

bool read_ad7991_batbrd(lion_current_batch batch1, panelref_lref_batch batch2) {
	uint16_t results[4];
	bool gotMutex = read_ad7991_batbrd_precise(results);
	// results[0] = L2_SNS
	batch1[1] = truncate_16t(results[0], S_L_SNS);
	// results[1] = L1_SNS
	batch1[0] = truncate_16t(results[1], S_L_SNS);
	// results[2] = L_REF
	batch2[1] = truncate_16t(results[2], S_LREF);
	// results[3] = PANELREF
	batch2[0] = truncate_16t(results[3], S_PANELREF);
	return gotMutex;
}

void read_ad7991_ctrlbrd(ad7991_ctrlbrd_batch batch) {
	if (xSemaphoreTake(i2c_irpow_mutex, HARDWARE_MUTEX_WAIT_TIME_TICKS))
	{
		bool got_semaphore = enable_ir_pow_if_necessary();
		read_ad7991_ctrlbrd_unsafe(batch);
		disable_ir_pow_if_necessary(got_semaphore);
		xSemaphoreGive(i2c_irpow_mutex);
	} else {
		log_error(ELOC_AD7991_CBRD, ECODE_I2C_IRPOW_MUTEX_TIMEOUT, false);
		memset(batch, 0, sizeof(ad7991_ctrlbrd_batch));
	}
}

/************************************************************************/
/* FLASH-RELATED FUNCTIONS - include unsafe and safe version            */
/************************************************************************/
// note: only called from flash_task, and with i2c_irpow_mutex held
// REQUIRES i2c_irpow_mutex, processor_adc_mutex AND 5V regulator enabled
void _read_led_temps_batch_unsafe(led_temps_batch batch, bool flashing_now) {
	for (int i = 4; i < 8; i++) {
		uint8_t rs8;
		status_code_genare_t sc = LTC1380_channel_select(TEMP_MULTIPLEXER_I2C, i, &rs8);
		log_if_error(TEMP_ELOCS[i], sc, true);
		commands_read_adc_mV_truncate(&rs8, P_AI_TEMP_OUT, TEMP_ELOCS[i], flashing_now ? S_LED_TEMP_FLASH : S_LED_TEMP_REG, true);
		batch[i - 4] = rs8;
	}
}

// REQUIRES i2c_irpow_mutex, processor_adc_mutex AND 5V regulator enabled
void _read_lifepo_temps_batch_unsafe(lifepo_bank_temps_batch batch) {
	for (int i = 0; i < 2; i++) {
		uint8_t rs8;
		status_code_genare_t sc = LTC1380_channel_select(TEMP_MULTIPLEXER_I2C, i, &rs8);
		log_if_error(TEMP_ELOCS[i], sc, true);
		commands_read_adc_mV_truncate(&rs8, P_AI_TEMP_OUT, TEMP_ELOCS[i], S_L_TEMP, true);
		batch[i] = rs8;
	}
}

void _read_lifepo_current_batch_unsafe(lifepo_current_batch batch, bool flashing_now) {
	uint sns_sig = flashing_now ? S_LF_SNS_FLASH : S_LF_SNS_REG;
	uint o_sig = flashing_now ? S_LF_OSNS_FLASH : S_LF_OSNS_REG;
	commands_read_adc_mV_truncate(&batch[0], P_AI_LFB1SNS, ELOC_LFB1SNS, sns_sig, true);
	commands_read_adc_mV_truncate(&batch[1], P_AI_LFB1OSNS, ELOC_LFB1OSNS, o_sig, true);
	commands_read_adc_mV_truncate(&batch[2], P_AI_LFB2SNS, ELOC_LFB2SNS, sns_sig, true);
	commands_read_adc_mV_truncate(&batch[3], P_AI_LFB2OSNS, ELOC_LFB2OSNS, o_sig, true);
}

// only used in antenna deploy task
void read_lifepo_current_precise(uint16_t* val_1, uint16_t* val_2, uint16_t* val_3, uint16_t* val_4) {
	if (xSemaphoreTake(i2c_irpow_mutex, HARDWARE_MUTEX_WAIT_TIME_TICKS))
	{
		bool got_semaphore = enable_ir_pow_if_necessary();
		if (xSemaphoreTake(processor_adc_mutex, HARDWARE_MUTEX_WAIT_TIME_TICKS))
		{
			#ifndef EQUISIM_SIMULATE_BATTERIES
				commands_read_adc_mV(val_1, P_AI_LFB1SNS, ELOC_LFB1SNS, S_LF_SNS_REG, true, false);
				commands_read_adc_mV(val_2, P_AI_LFB1OSNS, ELOC_LFB1OSNS, S_LF_OSNS_REG, true, false);
				commands_read_adc_mV(val_3, P_AI_LFB2SNS, ELOC_LFB2SNS, S_LF_SNS_REG, true, false);
				commands_read_adc_mV(val_4, P_AI_LFB2OSNS, ELOC_LFB2OSNS, S_LF_OSNS_REG, true, false);
			#else
				equisim_read_lifepo_current_precise(val_1, val_2, val_3, val_4);
			#endif

			xSemaphoreGive(processor_adc_mutex);
		} else {
			log_error(ELOC_LFB1SNS, ECODE_PROC_ADC_MUTEX_TIMEOUT, false);
			memset(val_1, 0, sizeof(uint16_t));
			memset(val_2, 0, sizeof(uint16_t));
			memset(val_3, 0, sizeof(uint16_t));
			memset(val_4, 0, sizeof(uint16_t));
		}
		disable_ir_pow_if_necessary(got_semaphore);
		xSemaphoreGive(i2c_irpow_mutex);
	} else {
		log_error(ELOC_LFB1SNS, ECODE_I2C_IRPOW_MUTEX_TIMEOUT, false);
		memset(val_1, 0, sizeof(uint16_t));
		memset(val_2, 0, sizeof(uint16_t));
		memset(val_3, 0, sizeof(uint16_t));
		memset(val_4, 0, sizeof(uint16_t));
	}
}

// reads precise values; just a helper function
static void read_lifepo_volts_precise_unsafe(uint16_t* val_1, uint16_t* val_2, uint16_t* val_3, uint16_t* val_4, bool precise) {
	#ifndef EQUISIM_SIMULATE_BATTERIES
		// note: lifepo voltages will not vary enough during flash to warrant a separate bound for them
		commands_read_adc_mV(val_1, P_AI_LF1REF, ELOC_LF1REF, S_LF_VOLT, true, precise);
		commands_read_adc_mV(val_2, P_AI_LF2REF, ELOC_LF2REF, S_LF_VOLT, true, precise);
		commands_read_adc_mV(val_3, P_AI_LF3REF, ELOC_LF3REF, S_LF_VOLT, true, precise);
		commands_read_adc_mV(val_4, P_AI_LF4REF, ELOC_LF4REF, S_LF_VOLT, true, precise);

		*val_2 = *val_2 * 195 / 100;
		*val_4 = *val_4 * 195 / 100;
		*val_1 = (*val_1 * 387 / 100) - *val_2;
		*val_3 = (*val_3 * 387 / 100) - *val_4;
	#else
		equisim_read_lifepo_volts_precise(val_1, val_2, val_3, val_4);
	#endif
}

// reads precise values; mutex safe
bool read_lifepo_volts_precise(uint16_t* val_1, uint16_t* val_2, uint16_t* val_3, uint16_t* val_4, bool precise) {
	if (xSemaphoreTake(processor_adc_mutex, HARDWARE_MUTEX_WAIT_TIME_TICKS))
	{
		read_lifepo_volts_precise_unsafe(val_1, val_2, val_3, val_4, precise);
		xSemaphoreGive(processor_adc_mutex);
		return true;
	} else {
		log_error(ELOC_LF1REF, ECODE_PROC_ADC_MUTEX_TIMEOUT, false);
		memset(val_1, 0, sizeof(uint16_t));
		memset(val_2, 0, sizeof(uint16_t));
		memset(val_3, 0, sizeof(uint16_t));
		memset(val_4, 0, sizeof(uint16_t));
		return false;
	}
}

// reads truncated batch; called in flash task while mutex is held
void _read_lifepo_volts_batch_unsafe(lifepo_volts_batch batch) {
	uint16_t val_1_precise;
	uint16_t val_2_precise;
	uint16_t val_3_precise;
	uint16_t val_4_precise;

	read_lifepo_volts_precise_unsafe(&val_1_precise, &val_2_precise, &val_3_precise, &val_4_precise, false);

	batch[0] = truncate_16t(val_1_precise, S_LF_VOLT);
	batch[1] = truncate_16t(val_2_precise, S_LF_VOLT);
	batch[2] = truncate_16t(val_3_precise, S_LF_VOLT);
	batch[3] = truncate_16t(val_4_precise, S_LF_VOLT);
}

// reads truncated batch; mutex safe
void read_lifepo_volts_batch(lifepo_volts_batch batch) {
	if (xSemaphoreTake(processor_adc_mutex, HARDWARE_MUTEX_WAIT_TIME_TICKS))
	{
		_read_lifepo_volts_batch_unsafe(batch);
		xSemaphoreGive(processor_adc_mutex);
	} else {
		log_error(ELOC_LF1REF, ECODE_PROC_ADC_MUTEX_TIMEOUT, false);
		memset(batch, 0, sizeof(lifepo_volts_batch));
	}
}

void _read_led_current_batch_unsafe(led_current_batch batch, bool flashing_now) {
	uint sig = flashing_now ? S_LED_SNS_FLASH : S_LED_SNS_REG;
	commands_read_adc_mV_truncate(&batch[0], P_AI_LED1SNS, ELOC_LED1SNS, sig, true);
	commands_read_adc_mV_truncate(&batch[1], P_AI_LED2SNS, ELOC_LED2SNS, sig, true);
	commands_read_adc_mV_truncate(&batch[2], P_AI_LED3SNS, ELOC_LED3SNS, sig, true);
	commands_read_adc_mV_truncate(&batch[3], P_AI_LED4SNS, ELOC_LED4SNS, sig, true);
}

void verify_flash_readings(bool flashing_now) {
	// note: if this function happens to context switch into being flashing
	// on this line, and then comes back while flashing, the flash
	// task will have the mutex so we'll wait here until it's done
	// (the passed flash state will be valid even if a flash happens)
	if (xSemaphoreTake(i2c_irpow_mutex, HARDWARE_MUTEX_WAIT_TIME_TICKS))
	{
		bool got_semaphore = enable_ir_pow_if_necessary();
		if (xSemaphoreTake(processor_adc_mutex, HARDWARE_MUTEX_WAIT_TIME_TICKS))
		{
			uint8_t buffer[4]; // size of largest data type

			// read values to nowhere, making them check bounds for errors
			_set_5v_enable_unsafe(true); // required only for these three
			_read_led_temps_batch_unsafe(buffer, flashing_now);
			_read_lifepo_temps_batch_unsafe(buffer);
			_set_5v_enable_unsafe(false);
			_read_lifepo_current_batch_unsafe(buffer, flashing_now);
			_read_lifepo_volts_batch_unsafe(buffer);
			_read_led_current_batch_unsafe(buffer, flashing_now);

			xSemaphoreGive(processor_adc_mutex);
		} else {
			log_error(ELOC_LED1SNS, ECODE_PROC_ADC_MUTEX_TIMEOUT, false);
			// no data passed back that needs to be cleared
		}
		disable_ir_pow_if_necessary(got_semaphore);
		xSemaphoreGive(i2c_irpow_mutex);
	} else {
		log_error(ELOC_LED1SNS, ECODE_I2C_IRPOW_MUTEX_TIMEOUT, false);
		// no data passed back that needs to be cleared
	}
}

/************************************************************************/
/* END OF FLASH-RELATED FUNCTIONS                                       */
/************************************************************************/

//raw is in mV
uint8_t get_pdiode_two_bit_range(uint16_t raw) {
	if (raw < PDIODE_00_01) {
		return 0;
	} else if (raw >= PDIODE_00_01 && raw < PDIODE_01_10) {
		return 1;
	} else if (raw >= PDIODE_01_10 && raw < PDIODE_10_11) {
		return 2;
	} else if (raw >= PDIODE_10_11) {
		return 3;
	} else {
		return 4;
	}
}

void read_pdiode_batch(pdiode_batch* batch) {
	if (xSemaphoreTake(i2c_irpow_mutex, HARDWARE_MUTEX_WAIT_TIME_TICKS))
	{
		bool got_semaphore = enable_ir_pow_if_necessary();
		if (xSemaphoreTake(processor_adc_mutex, HARDWARE_MUTEX_WAIT_TIME_TICKS))
		{
			uint8_t rs;
			memset(batch, 0, sizeof(pdiode_batch));
			for (int i = 0; i < 6; i++) {
				uint16_t result;

				status_code_genare_t sc = LTC1380_channel_select(PHOTO_MULTIPLEXER_I2C, i, &rs);
				log_if_error(PD_ELOCS[i], sc, false);

				commands_read_adc_mV(&result, P_AI_PD_OUT, PD_ELOCS[i], S_PD, false, false);
				uint8_t two_bit_range = get_pdiode_two_bit_range(result);
				if (two_bit_range == 4) {
					// PD_ACCESS used as general photo diode indicator
					log_error(ELOC_PD_NEG_Z, ECODE_UNEXPECTED_CASE, false);
				} else {
					*batch |= (two_bit_range << (i*2));
				}
			}
			xSemaphoreGive(processor_adc_mutex);
		} else {
			log_error(ELOC_PD_POS_Y, ECODE_PROC_ADC_MUTEX_TIMEOUT, false);
			memset(batch, 0, sizeof(pdiode_batch));
		}
		disable_ir_pow_if_necessary(got_semaphore);
		xSemaphoreGive(i2c_irpow_mutex);
	} else {
		log_error(ELOC_PD_POS_Y, ECODE_I2C_IRPOW_MUTEX_TIMEOUT, false);
		memset(batch, 0, sizeof(pdiode_batch));
	}
}

void en_and_read_lion_temps_batch(lion_temps_batch batch) {
	if (xSemaphoreTake(i2c_irpow_mutex, HARDWARE_MUTEX_WAIT_TIME_TICKS))
	{
		bool got_semaphore = enable_ir_pow_if_necessary();
		if (xSemaphoreTake(processor_adc_mutex, HARDWARE_MUTEX_WAIT_TIME_TICKS))
		{
			_set_5v_enable_unsafe(true);
			verify_regulators_unsafe();

			for (int i = 2; i < 4; i++) {
				uint8_t rs8;
				status_code_genare_t sc = LTC1380_channel_select(TEMP_MULTIPLEXER_I2C, i, &rs8);
				log_if_error(TEMP_ELOCS[i], sc, true);
				commands_read_adc_mV_truncate(&rs8, P_AI_TEMP_OUT, TEMP_ELOCS[i], S_L_TEMP, true);
				batch[i - 2] = rs8;
			}

			_set_5v_enable_unsafe(false);
			xSemaphoreGive(processor_adc_mutex);
		} else {
			log_error(ELOC_TEMP_L_1, ECODE_PROC_ADC_MUTEX_TIMEOUT, false);
			memset(batch, 0, sizeof(lion_temps_batch));
		}
		disable_ir_pow_if_necessary(got_semaphore);
		xSemaphoreGive(i2c_irpow_mutex);
	} else {
		log_error(ELOC_TEMP_L_1, ECODE_I2C_IRPOW_MUTEX_TIMEOUT, false);
		memset(batch, 0, sizeof(lion_temps_batch));
	}
}

void read_accel_batch(accelerometer_batch accel_batch) {
	int16_t rs[3];
	status_code_genare_t sc;
	if (xSemaphoreTake(i2c_irpow_mutex, HARDWARE_MUTEX_WAIT_TIME_TICKS))
	{
		bool got_semaphore = enable_ir_pow_if_necessary();
		sc = MPU9250_read_acc(rs);
		disable_ir_pow_if_necessary(got_semaphore);
		xSemaphoreGive(i2c_irpow_mutex);
	} else {
		log_error(ELOC_IMU_ACC, ECODE_I2C_IRPOW_MUTEX_TIMEOUT, false);
		memset(accel_batch, 0, sizeof(accelerometer_batch));
		return;
	}

	log_if_error(ELOC_IMU_ACC, sc, false);
	for (int i = 0; i < 3; i++) {
		accel_batch[i] = truncate_16t(rs[i], S_ACCEL);
	}
}

void _read_gyro_batch_unsafe(gyro_batch gyr_batch) {
	int16_t rs[3];
	status_code_genare_t sc = MPU9250_read_gyro(rs);

	log_if_error(ELOC_IMU_GYRO, sc, false);
	for (int i = 0; i < 3; i++) {
		log_if_out_of_bounds(rs[i], S_GYRO, ELOC_IMU_GYRO, false);
		gyr_batch[i] = truncate_16t(rs[i], S_GYRO);
	}
}

void read_gyro_batch(gyro_batch gyr_batch) {
	int16_t rs[3];
	status_code_genare_t sc;
	if (xSemaphoreTake(i2c_irpow_mutex, HARDWARE_MUTEX_WAIT_TIME_TICKS))
	{
		// didn't enable it, we won't turn it off
		bool got_semaphore = enable_ir_pow_if_necessary();
		sc = MPU9250_read_gyro(rs);
		disable_ir_pow_if_necessary(got_semaphore);
		xSemaphoreGive(i2c_irpow_mutex);
	} else {
		log_error(ELOC_IMU_GYRO, ECODE_I2C_IRPOW_MUTEX_TIMEOUT, false);
		memset(gyr_batch, 0, sizeof(gyro_batch));
		return;
	}

	log_if_error(ELOC_IMU_GYRO, sc, false);
	for (int i = 0; i < 3; i++) {
		log_if_out_of_bounds(rs[i], S_GYRO, ELOC_IMU_GYRO, false);
		gyr_batch[i] = truncate_16t(rs[i], S_GYRO);
	}
}

void read_magnetometer_batch(magnetometer_batch batch) {
	int16_t rs[3];
	status_code_genare_t sc;
	if (xSemaphoreTake(i2c_irpow_mutex, HARDWARE_MUTEX_WAIT_TIME_TICKS))
	{
		bool got_semaphore = enable_ir_pow_if_necessary();
		//sc = MPU9250_read_mag(rs);
		sc = HMC5883L_readXYZ(rs);
		disable_ir_pow_if_necessary(got_semaphore);
		xSemaphoreGive(i2c_irpow_mutex);
	} else {
		log_error(ELOC_IMU_MAG, ECODE_I2C_IRPOW_MUTEX_TIMEOUT, false);
		memset(batch, 0, sizeof(magnetometer_batch));
		return;
	}

	log_if_error(ELOC_IMU_MAG, sc, false);
	for (int i = 0; i < 3; i++) {
		batch[i] = truncate_16t(rs[i], S_MAG);
	}
}

bool read_bat_charge_dig_sigs_batch(bat_charge_dig_sigs_batch* batch) {
	if (xSemaphoreTake(i2c_irpow_mutex, HARDWARE_MUTEX_WAIT_TIME_TICKS))
	{
		bool got_semaphore = enable_ir_pow_if_necessary();
		#ifndef EQUISIM_SIMULATE_BATTERIES
			status_code_genare_t sc = TCA9535_init(batch);
			log_if_error(ELOC_TCA, sc, true);
			// zero out the places we're going to overwrite
			// see order in Message Format spreadsheet
			*batch &= 0b1111001111110000;
			// fill in the new values we want
			*batch |= get_input(P_L1_RUN_CHG);
			*batch |= (get_input(P_L2_RUN_CHG)<<1);
			*batch |= (get_input(P_LF_B1_RUNCHG)<<2);
			*batch |= (get_input(P_LF_B2_RUNCHG)<<3);
			*batch |= (get_input(P_L1_DISG)<<10);
			*batch |= (get_input(P_L2_DISG)<<11);
		#else
			equisim_read_bat_charge_dig_sigs_batch(batch);
		#endif

		disable_ir_pow_if_necessary(got_semaphore);
		xSemaphoreGive(i2c_irpow_mutex);
		return true;
	} else {
		log_error(ELOC_TCA, ECODE_I2C_IRPOW_MUTEX_TIMEOUT, false);
		memset(batch, 0, sizeof(bat_charge_dig_sigs_batch));
		return false;
	}
}

void read_imu_temp_batch(imu_temp_batch* batch) {
	if (xSemaphoreTake(i2c_irpow_mutex, HARDWARE_MUTEX_WAIT_TIME_TICKS)) {
		bool got_semaphore = enable_ir_pow_if_necessary();
		int16_t buf;
		enum status_code sc = MPU9250_read_temp(&buf);
		log_if_error(ELOC_IMU_TEMP, sc, false);
		log_if_out_of_bounds(buf, S_IMU_TEMP, ELOC_IMU_TEMP, false);
		*batch = truncate_16t(buf, S_IMU_TEMP);
		disable_ir_pow_if_necessary(got_semaphore);
		xSemaphoreGive(i2c_irpow_mutex);
	} else {
		log_error(ELOC_IMU_TEMP, ECODE_I2C_IRPOW_MUTEX_TIMEOUT, false);
		memset(batch, 0, sizeof(magnetometer_batch));
	}
}

void read_radio_temp_batch(radio_temp_batch* batch) {
	*batch = truncate_16t(get_radio_temp_cached(), S_RAD_TEMP);
}

bool read_field_from_bcds(bat_charge_dig_sigs_batch batch, bcds_conversions_t shift) {
	return (batch >> shift) & 1;
}

uint16_t untruncate(uint8_t val, sig_id_t sig) {
	uint16_t u16 = ((uint16_t)val) << 8;
	return u16 / get_line_m_from_signal(sig) - get_line_b_from_signal(sig);
}
