/*
 * battery_charging_simulated_data.h
 *
 * Created: 2/10/2018 12:40:32 AM
 *  Author: rjha
 */ 
#include "rtos_tasks/rtos_tasks.h"

#ifndef BATTERY_CHARGING_SIMULATED_DATA_H_
#define BATTERY_CHARGING_SIMULATED_DATA_H_

extern int simulated_timestamp;
extern int simulated_state;
extern int simulated_curr_charging_filled_up;

void equisim_read_lion_volts_precise(uint16_t* val_1, uint16_t* val_2);
void equisim_read_lifepo_volts_precise(uint16_t* val_1, uint16_t* val_2, uint16_t* val_3, uint16_t* val_4);
void equisim_read_lion_current_precise(uint16_t* val_1, uint16_t* val_2);
void equisim_read_lifepo_current_precise(uint16_t* val_1, uint16_t* val_2, uint16_t* val_3, uint16_t* val_4);

#endif /* BATTERY_CHARGING_SIMULATED_DATA_H_ */