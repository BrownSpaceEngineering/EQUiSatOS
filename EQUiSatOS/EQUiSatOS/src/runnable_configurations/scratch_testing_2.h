#ifndef SCRATCH_TESTING_2_H_
#define SCRATCH_TESTING_2_H_

#include "run.h"

void flatsat_init(void);
float read_IR(void);
MPU9250Reading read_IMU(void);
float read_temp(void);
void led_flash(void);
void flatsat_run(void);

#endif /* SCRATCH_TESTING_2_H_ */