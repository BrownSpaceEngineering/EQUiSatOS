#include "flatsat.h"

/*
 * CHECK THE DATASHEET THERES TOO MANY VARIABLES IN THE MEMORY ADDRESS
 * THIS IS PROBABLY NOT CORRECT
 */
uint8_t CHIP_ADDR = 0b1010111

void flatsat_run(void) {
    configure_i2c_standard(SERCOM4);

    uint8_t memLocation[2] = {0x0, 0x0};

    writeDataToAddress(memLocation, 2, CHIP_ADDR, true);

    uint8_t buffer[1] = {0x42};

    readFromAddress(buffer, 1, CHIP_ADDR, true);

    buffer[0] = buffer[0] + 1;

    uint8_t outBuffer[3];

    outBuffer[0] = 0x0;
    outBuffer[1] = 0x0;
    outBuffer[2] = buffer[0];

    writeDataToAddress(outBuffer, 3, CHIP_ADDR, true);
}