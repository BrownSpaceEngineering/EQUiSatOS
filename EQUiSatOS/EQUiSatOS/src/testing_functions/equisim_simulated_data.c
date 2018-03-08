#include "equisim_simulated_data.h"

/************************************************************************/
/* Misc. testing variables                                              */
/************************************************************************/
int simulated_timestamp = 0;
int simulated_state = IDLE_NO_FLASH;
int simulated_curr_charging_filled_up = 0;


/************************************************************************/
/* Battery voltage/current plug ins                                     */
/************************************************************************/
void equisim_read_lion_volts_precise(uint16_t* val_1, uint16_t* val_2) {
	
}

void equisim_read_lifepo_volts_precise(uint16_t* val_1, uint16_t* val_2, uint16_t* val_3, uint16_t* val_4) {
	
}

void equisim_read_lion_current_precise(uint16_t* val_1, uint16_t* val_2) {
	
}

void equisim_read_lifepo_current_precise(uint16_t* val_1, uint16_t* val_2, uint16_t* val_3, uint16_t* val_4) {
	
}