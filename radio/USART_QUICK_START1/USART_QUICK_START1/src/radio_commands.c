//
//  main.c
//  Cubesat Radio Commands
//
//  Created by Samuel Kortchmar on 3/8/15.
//  Copyright (c) 2015 Samuel Kortchmar. All rights reserved.
//

#include <asf.h>
#include "radio_commands.h"
#include "radio.h"
//<><><<>><><>Helpers<><><<>><><>

uint8_t computeCheckSum(uint8_t cmd[], int dataLength) {
    uint8_t sum = 0;
    for(int i = 0; i < dataLength; i ++) {
        sum += cmd[i];
    }
    return ~sum;
}


/*
 * Adds SoH character and computes checksum.
 * WARNING: Assumes cmd[] has been initialized to dataLength + 2
 */
int padCmd(uint8_t cmd[], int dataLength) {
    cmd[0] = 0x01; //SoH byte
    cmd[dataLength + 1] = computeCheckSum(&cmd[1], dataLength);
    return 0;
}

void printByte(uint8_t byte) {
    printf("0x%02x\n", byte);
}

/*
 * Writes simple 4-byte commands of the form {SOH, CONTROL, DATA, CHECKSUM}
 * Automatically adds SoH character and computes checksum.
 */
int simpleCommand(uint8_t controlChar, uint8_t dataByte, Radio* r) {
    int dataLength = 2;
    uint8_t cmd[dataLength + 2]; //extra two for checksum and SoH
    cmd[1] = controlChar;
    cmd[2] = dataByte;
    
    padCmd(cmd, dataLength);
    
    //Send to Radio
    sendToRadio(cmd, sizeof(cmd), r);
    
    
    return 0;
    
}

int simpleResponse(int responseType, int responseCode, Radio *r) {
    //SoH, Status, Status Code, Checksum
    int responseLength = 1 + 3;
    uint8_t response[responseLength];

    if  (receiveFromRadio(response, responseLength, r)) {
        //error
        return 1;
    }
                                    //Status Code, Checksum
    if (computeCheckSum(&response[1], 2) != response[3]) {
        //bad checksum, data is corrupted.
        return 1;
    }

    return !(response[1] == responseType && response[2] == responseCode);
}


int sendToRadio(uint8_t cmd[], int size, Radio* r) {
    for (int i = 0; i < size; i++) {
        printByte(cmd[i]);
    }
    return 0;

    // return usart_write_buffer_wait(r->in, cmd, size);
}

int receiveFromRadio(uint8_t* rx_data, uint16_t length, Radio* r) {
    return usart_read_buffer_wait(r->out, rx_data, length);
}

//<><><<>><><>Commands<><><<>><><>

/*
 * 0x02 = 4.8 kbps
 */
int setLinkSpeed(Radio* r) {
    if (simpleCommand(0x05, 0x02, r)) {
        //error
    }

    return simpleResponse(0x85, 0x00, r);
}

int setProtocol(Radio* r) {
    if (simpleCommand(0x07, 0x02, r)) {

    }

    return simpleResponse(0x87, 0x00, r);
}

/*
 * 0x00 = GMSK
 * Note: after this command, you need to do a WARM RESET in order to
 * apply the change.
 */
int setModulationFormat(Radio* r) {
    if (simpleCommand(0x2B, 0x00, r)) {

    }

    return simpleResponse(0xAB, 0x00, r);
}

/*
 * Note: after this command, you need to do a WARM RESET in order to
 * apply the change.
 */
int setTxFrequencyByChannel(uint8_t channelNo, uint32_t freq, Radio* r) {
    //TODO: is this actually the max no. of channels?
    if (channelNo > 15) {
        return 1;
    }
    
    int dataLength = 6;

    uint8_t cmd[dataLength + 2]; //extra two for checksum and SoH
    cmd[1] = 0x37;
    cmd[2] = channelNo;
    cmd[3] = (freq >> 24);
    cmd[4] = (freq >> 16);
    cmd[5] = (freq >> 8);
    cmd[6] = freq;
    
    
    padCmd(cmd, dataLength);
    
    if (sendToRadio(cmd, sizeof(cmd), r)) {
        //error
    }

    return simpleResponse(0xb7, 0x00, r);
}

/*
 * Note: after this command, you need to do a WARM RESET in order to
 * apply the change.
 */
int setRxFrequencyByChannel(uint8_t channelNo, uint32_t freq, Radio* r) {
    //TODO: is this actually the max no. of channels?
    if (channelNo > 15) {
        return 1;
    }
    
    int dataLength = 6;

    uint8_t cmd[dataLength + 2]; //extra two for checksum and SoH
    cmd[1] = 0x39;
    cmd[2] = channelNo;
    cmd[3] = (freq >> 24);
    cmd[4] = (freq >> 16);
    cmd[5] = (freq >> 8);
    cmd[6] = freq;
    
    
    padCmd(cmd, dataLength);
    
    if (sendToRadio(cmd, sizeof(cmd), r)) {
        //handle error
    }

    return simpleResponse(0xb9, 0x00, r);
}


int program(Radio* r) {
    uint8_t cmd[] = {0x01, 0x1e, 0xe1};
    if (sendToRadio(cmd, 3, r)) {
        //error
    }

    return simpleResponse(0x9E, 0x00, r);
}

/*
 * resetType can be one of three:
 * 0x00 - cold reset, aka use saved config
 * 0x01 - warm reset, aka use current config
 * 0x02 - hard, reset cpu
 * 0x03 - hard monitor, stay in monitor, reset cpu
 */
int reset(uint8_t resetType, Radio* r) {
    if (resetType > 0x03) {
        return 1;
    }
    
    if (simpleCommand(0x1d, resetType, r)) {

    }

    return simpleResponse(0x9D, 0x00, r);
}

/*
 * 0x01 - (FEC-1 (PCC Protocols), FEC-2 ( SATEL))
 * NOTE: 0x09, 0x00 turns off FEC.
 */
int setForwardErrorConnection(Radio* r) {
    if (simpleCommand(0x09, 0x01, r)) {
        //error
    }

    return simpleResponse(0x81, 0x00, r);
}

/*
 * 0x02 - low sensitivity
 * 0x03 - off
 *
 */
int setSquelch(uint8_t sensitivity, Radio* r) {
    if ((sensitivity != 0x02) || (sensitivity != 0x03)) {
        return 1;
    }
    
    if (simpleCommand(0x29, sensitivity, r)) {
        //error
    }

    return simpleResponse(0xA9, 0x00, r);
}