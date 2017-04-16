#ifndef FLATSAT_H_
#define FLATSAT_H_

#include "run.h"

void flatsat_init(void);
void read_IR(uint16_t* buffer);
MPU9250Reading read_IMU(void);
float read_temp(void);
void led_flash(void);
void flatsat_run(void);

#define LEN_IR 1

#endif /* SCRATCH_TESTING_2_H_ */