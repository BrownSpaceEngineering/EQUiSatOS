#ifndef radio_commands_h
#define radio_commands_h
#include "radio.h"

uint8_t computeCheckSum(uint8_t cmd[], int dataLength);
int padCmd(uint8_t cmd[], int dataLength);
void printByte(uint8_t byte);
int simpleCommand(uint8_t controlChar, uint8_t dataByte, Radio* r);
int simpleResponse(int responseType, int responseCode, Radio *r);
int sendToRadio(uint8_t cmd[], int size, Radio* r);
int receiveFromRadio(uint8_t* rx_data, uint16_t length, Radio* r);
int setLinkSpeed(Radio* r);
int setProtocol(Radio* r);
int setModulationFormat(Radio* r);
int setTxFrequencyByChannel(uint8_t channelNo, uint32_t freq, Radio* r);
int setRxFrequencyByChannel(uint8_t channelNo, uint32_t freq, Radio* r);
int program(Radio* r);
int reset(uint8_t resetType, Radio* r);
int setForwardErrorConnection(Radio* r);
int setSquelch(uint8_t sensitivity, Radio* r);
int sendDataPacket(uint8_t destOrSource, uint8_t data[], uint8_t size, Radio* r);

#endif
