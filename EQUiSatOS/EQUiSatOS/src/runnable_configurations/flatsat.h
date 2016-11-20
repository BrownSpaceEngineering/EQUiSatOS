#ifndef FLATSAT_H_
#define FLATSAT_H_

#include "run.h"

void flatsat_init(void);
float read_IR(void);
MPU9250Reading read_IMU(void);
float read_temp(void);
void led_flash(void);
void flatsat_run(void);

#endif /* FLATSAT_H_ */