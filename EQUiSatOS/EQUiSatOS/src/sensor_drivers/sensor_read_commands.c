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
	i2c_mutex = xSemaphoreCreateMutexStatic(&_i2c_mutex_d);
	irpow_mutex = xSemaphoreCreateMutexStatic(&_irpow_mutex_d);
	processor_adc_mutex = xSemaphoreCreateMutexStatic(&_processor_adc_mutex_d);
}

/************************************************************************/
/* HELPERS                                                              */
/************************************************************************/
/* NOTE: the "batch" value passed into these functions are generally arrays, so are passed by reference */

void read_ad7991_ctrlbrd_unsafe(ad7991_ctrlbrd_batch batch);

uint8_t truncate_16t(uint16_t src, sig_id_t sig) {
	uint16_t m = get_line_m_from_signal(sig);
	int16_t b = get_line_b_from_signal(sig);
	return (((uint16_t)(src + b)) * m) >> 8;
}

static void log_if_out_of_bounds(uint16_t reading, sig_id_t sig, uint8_t eloc, bool priority) {
	uint16_t low = get_low_bound_from_signal(sig);
	uint16_t high = get_high_bound_from_signal(sig);
	if (reading <= low) {
		log_error(eloc, ECODE_READING_LOW, priority);
	} else if (reading >= high) {
		log_error(eloc, ECODE_READING_HIGH, priority);
	}
}

// note: processor ADC is locked externally to these methods for speed and for particular edge cases
static void commands_read_adc_mV(uint16_t* dest, uint8_t pin, uint8_t eloc, sig_id_t sig, bool priority) {
	status_code_genare_t sc = configure_adc(&adc_instance, pin);
	log_if_error(eloc, sc, priority);
	sc = read_adc_mV(adc_instance, dest);
	log_if_error(eloc, sc, priority);
	log_if_out_of_bounds(*dest, sig, eloc, priority);
}

static void commands_read_adc_mV_truncate(uint8_t* dest, int pin, uint8_t eloc, sig_id_t sig, bool priority) {
	uint16_t read;
	commands_read_adc_mV(&read, pin, eloc, sig, priority);
	*dest = truncate_16t(read, sig);
}

// unsafe becuse need i2c_mutex AND/OR processor_adc_mutex
bool _set_5v_enable_unsafe(bool on) {
	// note: to avoid chance of deadlock, any locks
	// of the i2c bus / processor adc mutex must be above this
	if (hardware_state_mutex_take())
	{
		set_output(on, P_5V_EN);
		get_hw_states()->rail_5v_enabled = on;
		hardware_state_mutex_give();
		// allow time to power up before someone uses it
		if (on) vTaskDelay(EN_5V_POWER_UP_DELAY_MS / portTICK_PERIOD_MS);
		return true;
	} else {
		log_error(ELOC_5V_REF, ECODE_HW_STATE_MUTEX_TIMEOUT, true);
		return false;
	}
}

// wrapper function to handle enabling IR power:
// If we're in low power this is protected by a mutex, but
// if we're in a normal mode we expect it to be on and this
// function will turn it on in an emergency
bool enable_ir_pow_if_necessary(void) {
	bool got_mutex = false; // default is that we didn't (try to) get mutex
	bool is_enabled = get_output(P_IR_PWR_CMD);
	if (!is_enabled) {
		if (low_power_active()) {
			// irpow mutex locked only in LOW_POWER
			got_mutex = xSemaphoreTake(irpow_mutex, HARDWARE_MUTEX_WAIT_TIME_TICKS);
			if (!got_mutex) {
				log_error(ELOC_IR_POW, ECODE_IRPOW_MUTEX_TIMEOUT, true);
			}
		}
		// no matter if we got the mutex, still turn it on (we won't turn it OFF)
		set_output(true, P_IR_PWR_CMD);
		vTaskDelay(IR_WAKE_DELAY_MS / portTICK_PERIOD_MS);
		
		#ifdef USE_STRICT_ASSERTIONS
			configASSERT(false);
		#endif
	}
	// if it's already enabled, carry on!
	return got_mutex;
}

// wrapper function to handle disabling IR power:
// If we're in low power we must give the mutex if taken, but
// if we're in normal mode we don't do anything here (leave IR power on)
void disable_ir_pow_if_necessary(bool got_mutex_on_en) {
	if (got_mutex_on_en) {
		xSemaphoreGive(irpow_mutex);
		if (low_power_active()) {
			// only turn off if we got the mutex (favor
			// leaving it on to shutting someone's data reading down
			// and screwing up their values)
			set_output(false, P_IR_PWR_CMD);
		}
	}
}

// disables IR power if it's still on in LOW_POWER and no one has the mutex
// run periodically by low power data task; reports errors if NOT expected on when was
void disable_ir_pow_if_should_be_off(bool expected_on) {
	if (xSemaphoreTake(irpow_mutex, HARDWARE_MUTEX_WAIT_TIME_TICKS)) {
		if (low_power_active()
			&& get_output(P_IR_PWR_CMD)) {
			// if we're in low power AND we have the mutex, AND IR power is still on, something
			// is wrong and we should turn it off
			set_output(false, P_IR_PWR_CMD);
			if (!expected_on) {
				log_error(ELOC_IR_POW, ECODE_INCONSISTENT_STATE, true);
				configASSERT(false); // this should never happen if mutex system is good
			}
		}
		xSemaphoreGive(irpow_mutex);
	} else {
		log_error(ELOC_IR_POW, ECODE_IRPOW_MUTEX_TIMEOUT, true);
	}
	// if it's not low power or we fail to get the mutex we can't turn it off
}

static void verify_regulators_unsafe(void) {
	struct hw_states* states;
	ad7991_ctrlbrd_batch batch;

	// only lock hardware state mutex while needed to act on state,
	// but long enough to ensure the state doesn't change in the middle of checking it
	if (hardware_state_mutex_take()) {
		read_ad7991_ctrlbrd_unsafe(batch);
		states = get_hw_states();
		hardware_state_mutex_give();
	} else {
		log_error(ELOC_VERIFY_REGS, ECODE_HW_STATE_MUTEX_TIMEOUT, true);
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
			break;
	}
	log_if_out_of_bounds(batch[1], state_3v6_sns, ELOC_AD7991_CBRD_3V6_SNS, true);
	// 5VREF is index 2
	log_if_out_of_bounds(batch[2], states->rail_5v_enabled ? S_5VREF_ON : S_5VREF_OFF, ELOC_AD7991_CBRD_5V_REF, true);
	// 3V3REF current is index 3
	log_if_out_of_bounds(batch[3], S_3V3_REF, ELOC_AD7991_CBRD_3V3_REF, true);
}

void verify_regulators(void) {
	if (xSemaphoreTake(i2c_mutex, HARDWARE_MUTEX_WAIT_TIME_TICKS))
	{
		// note: IR power will always be enabled if necessary but we can't
		// give an un-taken mutex
		bool got_irpow_mutex = enable_ir_pow_if_necessary();
		verify_regulators_unsafe();
		disable_ir_pow_if_necessary(got_irpow_mutex);
		xSemaphoreGive(i2c_mutex);
	} else {		
		log_error(ELOC_AD7991_CBRD_3V3_REF, ECODE_I2C_MUTEX_TIMEOUT, true);
	}
}

/************************************************************************/
/* SENSOR BATCH READING FUNCTIONS                                       */
/************************************************************************/

void read_ir_object_temps_batch(ir_object_temps_batch batch) {
	if (xSemaphoreTake(i2c_mutex, HARDWARE_MUTEX_WAIT_TIME_TICKS))
	{
		// note: IR power will always be enabled if necessary but we can't
		// give an un-taken mutex
		bool got_irpow_mutex = enable_ir_pow_if_necessary();
		for (int i = 0; i < 6; i ++) {
			uint16_t obj;
			status_code_genare_t sc = MLX90614_read_all_obj(IR_ADDS[i], &obj);
			log_if_error(IR_ELOCS[i], sc, false);
			log_if_out_of_bounds(obj, S_IR_OBJ, IR_ELOCS[i], false);
			batch[i] = obj;
		}
		disable_ir_pow_if_necessary(got_irpow_mutex);

		xSemaphoreGive(i2c_mutex);
	} else {
		log_error(ELOC_IR_POS_Z, ECODE_I2C_MUTEX_TIMEOUT, true);
		memset(batch, 0, sizeof(ir_object_temps_batch));
	}
}

void read_ir_ambient_temps_batch(ir_ambient_temps_batch batch) {
	if (xSemaphoreTake(i2c_mutex, HARDWARE_MUTEX_WAIT_TIME_TICKS))
	{
		// note: IR power will always be enabled if necessary but we can't
		// give an un-taken mutex
		bool got_irpow_mutex = enable_ir_pow_if_necessary();
		for (int i = 0; i < 6; i++) {
			uint16_t amb;
			status_code_genare_t sc = MLX90614_read2ByteValue(IR_ADDS[i], AMBIENT, &amb);
			log_if_error(IR_ELOCS[i], sc, false);
			log_if_out_of_bounds(amb, S_IR_AMB, IR_ELOCS[i], false);
			batch[i] = truncate_16t(amb, S_IR_AMB);
		}
		disable_ir_pow_if_necessary(got_irpow_mutex);
		xSemaphoreGive(i2c_mutex);
	} else {
		log_error(ELOC_IR_POS_Y, ECODE_I2C_MUTEX_TIMEOUT, true);
		memset(batch, 0, sizeof(ir_ambient_temps_batch));
	}
}

// TODO: make sure this all looks okay
void read_lion_volts_batch(lion_volts_batch batch) {
	uint16_t val_1_precise;
	uint16_t val_2_precise;

	// locks and releases processor_adc_mutex
	read_lion_volts_precise(&val_1_precise, &val_2_precise);
	batch[0] = truncate_16t(val_1_precise, S_L_VOLT);
	batch[1] = truncate_16t(val_2_precise, S_L_VOLT);
}

void read_lion_volts_precise(uint16_t* val_1, uint16_t* val_2) {
	if (xSemaphoreTake(processor_adc_mutex, HARDWARE_MUTEX_WAIT_TIME_TICKS))
	{
		#ifndef EQUISIM_SIMULATE_BATTERIES
			commands_read_adc_mV(val_1, P_AI_L1_REF, ELOC_L1_REF, S_L_VOLT, true);
			commands_read_adc_mV(val_2, P_AI_L2_REF, ELOC_L2_REF, S_L_VOLT, true);
			*val_1 = *val_1 * 25 / 10;
			*val_2 = *val_2 * 25 / 10;
		#else
			equisim_read_lion_volts_precise(val_1, val_2);
		#endif

		xSemaphoreGive(processor_adc_mutex);
	} else {
		log_error(ELOC_L1_REF, ECODE_PROC_ADC_MUTEX_TIMEOUT, true);
		memset(val_1, 0, sizeof(uint16_t));
		memset(val_2, 0, sizeof(uint16_t));
	}
}

void read_ad7991_batbrd(lion_current_batch batch1, panelref_lref_batch batch2) {
	uint16_t results[4];
	status_code_genare_t sc;
	sig_id_t sig;

	// (we need to lock i2c_irpow_mutex before hardware_state_mutex to avoid deadlock)
	if (xSemaphoreTake(i2c_mutex, HARDWARE_MUTEX_WAIT_TIME_TICKS)) {
		// note: IR power will always be enabled if necessary but we can't
		// give an un-taken mutex
		bool got_irpow_mutex = enable_ir_pow_if_necessary();
		// only lock hardware state mutex while needed to act on state,
		// but long enough to ensure the state doesn't change in the middle of checking it
		if (hardware_state_mutex_take()) {
			sc = AD7991_read_all_mV(results, AD7991_BATBRD);
			log_if_error(ELOC_AD7991_BBRD, sc, true);

			struct hw_states* states = get_hw_states();
			if (states->antenna_deploying) {
				sig = S_L_SNS_ANT_DEPLOY;
			} else if (states->radio_state == RADIO_TRANSMITTING) {
				sig = S_L_SNS_TRANSMIT;
			} else if (states->radio_state == RADIO_IDLE) {
				sig = S_L_SNS_IDLE_RAD_ON;
			} else {
				sig = S_L_SNS_IDLE_RAD_OFF;
			}

			hardware_state_mutex_give();
		} else {
			log_error(ELOC_AD7991_BBRD, ECODE_HW_STATE_MUTEX_TIMEOUT, true);
			memset(batch1, 0, sizeof(lion_current_batch));
			memset(batch2, 0, sizeof(panelref_lref_batch));
			xSemaphoreGive(i2c_mutex); // outer mutex
			return;
		}
		disable_ir_pow_if_necessary(got_irpow_mutex);
		xSemaphoreGive(i2c_mutex);
	} else {
		log_error(ELOC_AD7991_BBRD, ECODE_I2C_MUTEX_TIMEOUT, true);
		memset(batch1, 0, sizeof(lion_current_batch));
		memset(batch2, 0, sizeof(panelref_lref_batch));
		return;
	}

	// results[0] = L2_SNS
	batch1[1] = truncate_16t(results[0], S_L_SNS);
	log_if_out_of_bounds(results[0], sig, ELOC_AD7991_BBRD_L2_SNS, true);
	// results[1] = L1_SNS
	batch1[0] = truncate_16t(results[1], S_L_SNS);
	log_if_out_of_bounds(results[1], sig, ELOC_AD7991_BBRD_L1_SNS, true);

	// results[2] = L_REF
	batch2[1] = truncate_16t(results[2], S_LREF);
	log_if_out_of_bounds(results[2], S_LREF, ELOC_AD7991_BBRD_L1_SNS, true);
	// results[3] = PANELREF
	#ifdef EQUISIM_SIMULATE_BATTERIES
		results[3] = equisim_read_panelref();
	#endif
	batch2[0] = truncate_16t(results[3], S_PANELREF);
	log_if_out_of_bounds(results[3], S_PANELREF, ELOC_AD7991_BBRD_L2_SNS, true);
}

void read_lion_current_precise(uint16_t* val_1, uint16_t* val_2) {
	uint16_t results[4];
	sig_id_t sig;
	status_code_genare_t sc;

	// (we need to lock i2c_irpow_mutex before hardware_state_mutex to avoid deadlock)
	if (xSemaphoreTake(i2c_mutex, HARDWARE_MUTEX_WAIT_TIME_TICKS)) {
		// note: IR power will always be enabled if necessary but we can't
		// give an un-taken mutex
		bool got_irpow_mutex = enable_ir_pow_if_necessary();
		// only lock hardware state mutex while needed to act on state,
		// but long enough to ensure the state doesn't change in the middle of checking it
		if (hardware_state_mutex_take()) {
			#ifndef EQUISIM_SIMULATE_BATTERIES
				sc = AD7991_read_all_mV(results, AD7991_BATBRD);
				log_if_error(ELOC_AD7991_BBRD, sc, true);
			#else
				equisim_read_lion_current_precise(val_1, val_2);
			#endif

			struct hw_states* states = get_hw_states();
			if (states->antenna_deploying) {
				sig = S_L_SNS_ANT_DEPLOY;
			} else if (states->radio_state == RADIO_TRANSMITTING) {
				sig = S_L_SNS_TRANSMIT;
			} else if (states->radio_state == RADIO_IDLE) {
				sig = S_L_SNS_IDLE_RAD_ON;
			} else {
				sig = S_L_SNS_IDLE_RAD_OFF;
			}

			hardware_state_mutex_give();
		} else {
			log_error(ELOC_AD7991_BBRD, ECODE_HW_STATE_MUTEX_TIMEOUT, true);
			xSemaphoreGive(i2c_mutex); // outer mutex
			return;
		}
		disable_ir_pow_if_necessary(got_irpow_mutex);
		xSemaphoreGive(i2c_mutex);
	} else {
		log_error(ELOC_AD7991_BBRD, ECODE_I2C_MUTEX_TIMEOUT, true);
		return;
	}

	#ifndef EQUISIM_SIMULATE_BATTERIES
	// results[1] = L1_SNS
	*val_1 = results[1];
	log_if_out_of_bounds(*val_1, sig, ELOC_AD7991_BBRD_L1_SNS, true);
	// results[0] = L2_SNS
	*val_2 = results[0];
	log_if_out_of_bounds(*val_2, sig, ELOC_AD7991_BBRD_L2_SNS, true);
	#endif
}

// TODO: why is ad7991_ctrlbrd required outside here??
// unsafe version required for verify_regulators_unsafe
void read_ad7991_ctrlbrd_unsafe(ad7991_ctrlbrd_batch batch) {
	status_code_genare_t sc = AD7991_read_all_mV(batch, AD7991_CTRLBRD);
	log_if_error(ELOC_AD7991_CBRD, sc, false);
}

void read_ad7991_ctrlbrd(ad7991_ctrlbrd_batch batch) {
	if (xSemaphoreTake(i2c_mutex, HARDWARE_MUTEX_WAIT_TIME_TICKS))
	{
		// note: IR power will always be enabled if necessary but we can't
		// give an un-taken mutex
		bool got_irpow_mutex = enable_ir_pow_if_necessary();
		read_ad7991_ctrlbrd_unsafe(batch);
		disable_ir_pow_if_necessary(got_irpow_mutex);
		xSemaphoreGive(i2c_mutex);
	} else {
		log_error(ELOC_AD7991_CBRD, ECODE_I2C_MUTEX_TIMEOUT, true);
		memset(batch, 0, sizeof(ad7991_ctrlbrd_batch));
	}
}

/************************************************************************/
/* FLASH-RELATED FUNCTIONS - include unsafe and safe version            */
/************************************************************************/
// note: only called from flash_task, and with i2c_irpow_mutex held
void _read_led_temps_batch_unsafe(led_temps_batch batch) {
	for (int i = 4; i < 8; i++) {
		uint8_t rs8;
		status_code_genare_t sc = LTC1380_channel_select(TEMP_MULTIPLEXER_I2C, i, &rs8);
		log_if_error(TEMP_ELOCS[i], sc, true);
		commands_read_adc_mV_truncate(&rs8, P_AI_TEMP_OUT, TEMP_ELOCS[i], S_LED_TEMP, true);
		batch[i - 4] = rs8;
	}
}

void _read_lifepo_temps_batch_unsafe(lifepo_bank_temps_batch batch) {
	for (int i = 0; i < 2; i++) {
		uint8_t rs8;
		status_code_genare_t sc = LTC1380_channel_select(TEMP_MULTIPLEXER_I2C, i, &rs8);
		log_if_error(TEMP_ELOCS[i], sc, true);
		// TODO: define different 'on' range?
		commands_read_adc_mV_truncate(&rs8, P_AI_TEMP_OUT, TEMP_ELOCS[i], S_L_TEMP, true);
		batch[i] = rs8;
	}
}

// TODO: only reason the second argument exists right now is because of system_test
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
	if (xSemaphoreTake(i2c_mutex, HARDWARE_MUTEX_WAIT_TIME_TICKS))
	{
		// note: IR power will always be enabled if necessary but we can't
		// give an un-taken mutex
		bool got_irpow_mutex = enable_ir_pow_if_necessary();
		if (xSemaphoreTake(processor_adc_mutex, HARDWARE_MUTEX_WAIT_TIME_TICKS))
		{
			#ifndef EQUISIM_SIMULATE_BATTERIES
				commands_read_adc_mV(val_1, P_AI_LFB1SNS, ELOC_LFB1SNS, S_LF_SNS_REG, true);
				commands_read_adc_mV(val_2, P_AI_LFB1OSNS, ELOC_LFB1OSNS, S_LF_OSNS_REG, true);
				commands_read_adc_mV(val_3, P_AI_LFB2SNS, ELOC_LFB2SNS, S_LF_SNS_REG, true);
				commands_read_adc_mV(val_3, P_AI_LFB2OSNS, ELOC_LFB2OSNS, S_LF_OSNS_REG, true);
			#else
				equisim_read_lifepo_current_precise(val_1, val_2, val_3, val_4);
			#endif

			xSemaphoreGive(processor_adc_mutex);
		} else {
			log_error(ELOC_LFB1SNS, ECODE_PROC_ADC_MUTEX_TIMEOUT, true);
		}
		disable_ir_pow_if_necessary(got_irpow_mutex);
		xSemaphoreGive(i2c_mutex);
	} else {
		log_error(ELOC_LFB1SNS, ECODE_I2C_MUTEX_TIMEOUT, true);
	}
}

// reads precise values; just a helper function
static void read_lifepo_volts_precise_unsafe(uint16_t* val_1, uint16_t* val_2, uint16_t* val_3, uint16_t* val_4) {
	#ifndef EQUISIM_SIMULATE_BATTERIES
		// note: lifepo voltages will not vary enough during flash to warrant a separate bound for them
		commands_read_adc_mV(val_1, P_AI_LF1REF, ELOC_LF1REF, S_LF_VOLT, true);
		commands_read_adc_mV(val_2, P_AI_LF2REF, ELOC_LF2REF, S_LF_VOLT, true);
		commands_read_adc_mV(val_3, P_AI_LF3REF, ELOC_LF3REF, S_LF_VOLT, true);
		commands_read_adc_mV(val_4, P_AI_LF4REF, ELOC_LF4REF, S_LF_VOLT, true);

		*val_2 = *val_2 * 195 / 100;
		*val_4 = *val_4 * 195 / 100;
		*val_1 = (*val_1 * 387 / 100) - *val_2;
		*val_3 = (*val_3 * 387 / 100) - *val_4;
	#else
		equisim_read_lifepo_volts_precise(val_1, val_2, val_3, val_4);
	#endif
}

// reads precise values; mutex safe
void read_lifepo_volts_precise(uint16_t* val_1, uint16_t* val_2, uint16_t* val_3, uint16_t* val_4) {
	if (xSemaphoreTake(processor_adc_mutex, HARDWARE_MUTEX_WAIT_TIME_TICKS))
	{
		read_lifepo_volts_precise_unsafe(val_1, val_2, val_3, val_4);
		xSemaphoreGive(processor_adc_mutex);
	} else {
		log_error(ELOC_L1_REF, ECODE_PROC_ADC_MUTEX_TIMEOUT, true);
		memset(val_1, 0, sizeof(uint16_t));
		memset(val_2, 0, sizeof(uint16_t));
		memset(val_3, 0, sizeof(uint16_t));
		memset(val_4, 0, sizeof(uint16_t));
	}
}

// reads truncated batch; called in flash task while mutex is held
void _read_lifepo_volts_batch_unsafe(lifepo_volts_batch batch) {
	uint16_t val_1_precise;
	uint16_t val_2_precise;
	uint16_t val_3_precise;
	uint16_t val_4_precise;

	read_lifepo_volts_precise_unsafe(&val_1_precise, &val_2_precise, &val_3_precise, &val_4_precise);

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
		log_error(ELOC_LF1REF, ECODE_PROC_ADC_MUTEX_TIMEOUT, true);
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
	if (xSemaphoreTake(i2c_mutex, HARDWARE_MUTEX_WAIT_TIME_TICKS))
	{
		// note: IR power will always be enabled if necessary but we can't
		// give an un-taken mutex
		bool got_irpow_mutex = enable_ir_pow_if_necessary();
		if (xSemaphoreTake(processor_adc_mutex, HARDWARE_MUTEX_WAIT_TIME_TICKS))
		{
			uint8_t buffer[4]; // size of largest data type

			// read values to nowhere, making them check bounds for errors
			_set_5v_enable_unsafe(true); // required only for these three
			_read_led_temps_batch_unsafe(buffer);
			_read_lifepo_temps_batch_unsafe(buffer);
			_set_5v_enable_unsafe(false);
			_read_lifepo_current_batch_unsafe(buffer, flashing_now);
			_read_lifepo_volts_batch_unsafe(buffer);
			_read_led_current_batch_unsafe(buffer, flashing_now);

			xSemaphoreGive(processor_adc_mutex);
		} else {
			log_error(ELOC_LED1SNS, ECODE_PROC_ADC_MUTEX_TIMEOUT, true);
			// no data passed back that needs to be cleared
		}
		disable_ir_pow_if_necessary(got_irpow_mutex);
		xSemaphoreGive(i2c_mutex);
	} else {
		log_error(ELOC_LED1SNS, ECODE_I2C_MUTEX_TIMEOUT, true);
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
	if (xSemaphoreTake(i2c_mutex, HARDWARE_MUTEX_WAIT_TIME_TICKS))
	{
		// note: IR power will always be enabled if necessary but we can't
		// give an un-taken mutex
		bool got_irpow_mutex = enable_ir_pow_if_necessary();
		if (xSemaphoreTake(processor_adc_mutex, HARDWARE_MUTEX_WAIT_TIME_TICKS))
		{
			uint8_t rs;
			memset(batch, 0, sizeof(pdiode_batch));
			for (int i = 0; i < 6; i++) {
				uint16_t result;

				status_code_genare_t sc = LTC1380_channel_select(PHOTO_MULTIPLEXER_I2C, i, &rs);
				log_if_error(PD_ELOCS[i], sc, false);
				
				commands_read_adc_mV(&result, P_AI_PD_OUT, PD_ELOCS[i], S_PD, false);
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
			log_error(ELOC_PD_POS_Y, ECODE_PROC_ADC_MUTEX_TIMEOUT, true);
		}
		disable_ir_pow_if_necessary(got_irpow_mutex);
		xSemaphoreGive(i2c_mutex);
	} else {
		log_error(ELOC_PD_POS_Y, ECODE_I2C_MUTEX_TIMEOUT, true);
	}
}

void en_and_read_lion_temps_batch(lion_temps_batch batch) {
	if (xSemaphoreTake(i2c_mutex, HARDWARE_MUTEX_WAIT_TIME_TICKS))
	{
		// note: IR power will always be enabled if necessary but we can't
		// give an un-taken mutex
		bool got_irpow_mutex = enable_ir_pow_if_necessary();
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
			log_error(ELOC_TEMP_L_1, ECODE_PROC_ADC_MUTEX_TIMEOUT, true);
			memset(batch, 0, sizeof(lion_temps_batch));
		}
		disable_ir_pow_if_necessary(got_irpow_mutex);
		xSemaphoreGive(i2c_mutex);
	} else {
		log_error(ELOC_TEMP_L_1, ECODE_I2C_MUTEX_TIMEOUT, true);
		memset(batch, 0, sizeof(lion_temps_batch));
	}
}

void read_accel_batch(accelerometer_batch accel_batch) {
	int16_t rs[3];
	status_code_genare_t sc;
	if (xSemaphoreTake(i2c_mutex, HARDWARE_MUTEX_WAIT_TIME_TICKS))
	{
		// note: IR power will always be enabled if necessary but we can't
		// give an un-taken mutex
		bool got_irpow_mutex = enable_ir_pow_if_necessary();
		sc = MPU9250_read_acc(rs);
		disable_ir_pow_if_necessary(got_irpow_mutex);
		xSemaphoreGive(i2c_mutex);
	} else {
		log_error(ELOC_IMU_ACC, ECODE_I2C_MUTEX_TIMEOUT, true);
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
	if (xSemaphoreTake(i2c_mutex, HARDWARE_MUTEX_WAIT_TIME_TICKS))
	{
		// note: IR power will always be enabled if necessary but we can't
		// give an un-taken mutex
		bool got_irpow_mutex = enable_ir_pow_if_necessary();
		sc = MPU9250_read_gyro(rs);
		disable_ir_pow_if_necessary(got_irpow_mutex);
		xSemaphoreGive(i2c_mutex);
	} else {
		log_error(ELOC_IMU_GYRO, ECODE_I2C_MUTEX_TIMEOUT, true);
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
	if (xSemaphoreTake(i2c_mutex, HARDWARE_MUTEX_WAIT_TIME_TICKS))
	{
		enable_ir_pow_if_necessary();
		//sc = MPU9250_read_mag(rs);
		sc = HMC5883L_readXYZ(rs);
		xSemaphoreGive(i2c_mutex);
	} else {
		log_error(ELOC_IMU_MAG, ECODE_I2C_MUTEX_TIMEOUT, true);
		memset(batch, 0, sizeof(magnetometer_batch));
		return;
	}

	log_if_error(ELOC_IMU_MAG, sc, false);
	for (int i = 0; i < 3; i++) {
		batch[i] = truncate_16t(rs[i], S_MAG);
	}
}

void read_bat_charge_dig_sigs_batch(bat_charge_dig_sigs_batch* batch) {
	status_code_genare_t sc;
	if (xSemaphoreTake(i2c_mutex, HARDWARE_MUTEX_WAIT_TIME_TICKS))
	{
		// note: IR power will always be enabled if necessary but we can't
		// give an un-taken mutex
		bool got_irpow_mutex = enable_ir_pow_if_necessary();
		#ifndef EQUISIM_SIMULATE_BATTERIES
			sc = TCA9535_init(batch);
			// zero out the places we're going to overwrite
			// see order in Message Format spreadsheet
			*batch &= 0xF3F0;
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

		disable_ir_pow_if_necessary(got_irpow_mutex);
		xSemaphoreGive(i2c_mutex);
	} else {
		log_error(ELOC_TCA, ECODE_I2C_MUTEX_TIMEOUT, true);
		memset(batch, 0, sizeof(bat_charge_dig_sigs_batch));
	}
	log_if_error(ELOC_TCA, sc, true);
}

void read_imu_temp_batch(imu_temp_batch* batch) {
	if (xSemaphoreTake(i2c_mutex, HARDWARE_MUTEX_WAIT_TIME_TICKS)) {
		// note: IR power will always be enabled if necessary but we can't
		// give an un-taken mutex
		bool got_irpow_mutex = enable_ir_pow_if_necessary();
		int16_t buf;
		enum status_code sc = MPU9250_read_temp(&buf);
		log_if_error(ELOC_IMU_TEMP, sc, false);
		log_if_out_of_bounds(buf, S_IMU_TEMP, ELOC_IMU_TEMP, false);
		*batch = truncate_16t(buf, S_IMU_TEMP);
		disable_ir_pow_if_necessary(got_irpow_mutex);
		xSemaphoreGive(i2c_mutex);
	} else {
		log_error(ELOC_IMU_TEMP, ECODE_I2C_MUTEX_TIMEOUT, true);
		memset(batch, 0, sizeof(magnetometer_batch));
	}
}

void read_radio_temp_batch(radio_temp_batch* batch) {
	*batch = truncate_16t(get_radio_temp_cached(), S_RAD_TEMP);
}

bool read_field_from_bcds(bat_charge_dig_sigs_batch batch, bcds_conversions_t shift) {
	return (batch >> shift) & 1;
}
