//
//  main.c
//  Cubesat Radio Commands
//
//  Created by Samuel Kortchmar on 3/8/15.
//  Copyright (c) 2015 Samuel Kortchmar. All rights reserved.
//

#include <stdio.h>
#include <stdint.h>

uint8_t computeCheckSum(uint8_t cmd[], int dataLength);
int simpleCommand(uint8_t controlChar, uint8_t dataByte);
int setChannel(uint8_t channelNo);
void printByte(uint8_t byte);
int setTxFrequencyByChannel(uint8_t channelNo, uint32_t freq);


int main(int argc, const char * argv[]) {
    setTxFrequencyByChannel(15, 4800);
    return 0;
}
//<><><<>><><>Helpers<><><<>><><>\\

//We use chars because 1 char = 1 byte
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
int simpleCommand(uint8_t controlChar, uint8_t dataByte) {
    int dataLength = 2;
    uint8_t cmd[dataLength + 2]; //extra two for checksum and SoH
    cmd[1] = controlChar;
    cmd[2] = dataByte;
    
    padCmd(cmd, dataLength);
    
    //Send to Radio
    //    usart_write_buffer_wait(&usart_instance, cmd, sizeof(cmd));
    
    
    printByte(cmd[0]);
    printByte(cmd[1]);
    printByte(cmd[2]);
    printByte(cmd[3]);
    
    return 0;
    
}

//<><><<>><><>Commands<><><<>><><>\\

/*
 * 0x02 = 4.8 kbps
 */
int setLinkSpeed() {
    return simpleCommand(0x05, 0x02);
}

int setProtocol() {
    return simpleCommand(0x07, 0x02);
}

/*
 * 0x00 = GMSK
 */
int setModulationFormat() {
    return simpleCommand(0x02, 0x00);
}

int setTxFrequencyByChannel(uint8_t channelNo, uint32_t freq) {
    //TODO: is this actually the max no. of channels?
    if (channelNo > 15) {
        return 1;
    }
    
    int dataLength = 5;

    uint8_t cmd[dataLength + 2]; //extra two for checksum and SoH
    cmd[1] = channelNo;
    cmd[2] = (freq >> 24);
    cmd[3] = (freq >> 16);
    cmd[4] = (freq >> 8);
    cmd[5] = freq;
    
    
    padCmd(cmd, dataLength);
    
    //Send to Radio
    //    usart_write_buffer_wait(&usart_instance, cmd, sizeof(cmd));
    
    
    printByte(cmd[0]);
    printByte(cmd[1]);
    printByte(cmd[2]);
    printByte(cmd[3]);
    printByte(cmd[4]);
    printByte(cmd[5]);
    printByte(cmd[6]);
    
    return 0;
    
}

//
//int setRxFrequencyByChannel() {
//
//}
//

int program() {
    uint8_t cmd[] = {0x01, 0x1e, 0xe1};
    printByte(cmd[0]);
    printByte(cmd[1]);
    printByte(cmd[2]);
    return 1;
}

/*
 * resetType can be one of three:
 * 0x00 - cold reset, aka use saved config
 * 0x01 - warm reset, aka use current config
 * 0x02 - hard, reset cpu
 * 0x03 - hard monitor, stay in monitor, reset cpu
 */
int reset(uint8_t resetType) {
    if (resetType > 0x03) {
        return 1;
    }
    
    return simpleCommand(0x1d, resetType);
}

/*
 * 0x01 - (FEC-1 (PCC Protocols), FEC-2 ( SATEL))
 * NOTE: 0x09, 0x00 turns off FEC.
 */
int setForwardErrorConnection() {
    return simpleCommand(0x09, 0x01);
}

/*
 * 0x02 - low sensitivity
 * 0x03 - off
 *
 */
int setSquelch(uint8_t sensitivity) {
    if ((sensitivity != 0x02) || (sensitivity != 0x03)) {
        return 1;
    }
    
    return simpleCommand(0x29, sensitivity);
}