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
uint8_t readFromADC(enum adc_positive_input pin);
float convertToVoltage(uint8_t reading);

#define LEN_IR 6
#define ARDUINO_ADDR 0x08
#define LEN_ADC 4 // Should be 14

#endif /* SCRATCH_TESTING_2_H_ */