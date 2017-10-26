#ifndef FLATSAT_H_
#define FLATSAT_H_

#include "../global.h"

void flatsat_init(void);
void read_IR(uint16_t* buffer);
MPU9250Reading read_IMU(void);
float read_temp(void);
void led_flash(void);
void flatsat_run(void);
void sendToArduino(uint8_t* data, uint8_t length);
float convertToVoltage(uint16_t reading);
void readRemoteADC_0(float* batReadings);
void readRemoteADC_1(float* cntrlReadings);
#define LEN_IR 6
#define ARDUINO_ADDR 0x08
#define LEN_ADC 14 // Should be 14

#endif /* SCRATCH_TESTING_2_H_ */