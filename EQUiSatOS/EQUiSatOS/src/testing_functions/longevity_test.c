/* Longevity testing involves polling sensors for data, and writing output
   to the OpenLog (via SERCOM)

   Author: Arun
   Date: 24.04.2016

   Sensors to poll:
     - IR sensor
        * Raw IR channels 1 and 2
        * Ambient temperature
        * Object temperature
     - Temperature sensor
        * Ambient temperature
     - Magnetometer
        * Read magnetic field strengths in x,y,z directions
        * Get compass heading (from xyz)
     - Photodiode
        * Read brightness in relative voltage
*/
#include <stdarg.h>
#include "main.h"

#define STR_LEN 128

const char* IRSensor_format = "MLX90614: (%i, %i), %f, %f\n";
const char* TempSensor_format = "TEMD6200: %f\n";
const char* Magnetometer_format = "HMC5883L: (%i, %i, %i), %f\n";
const char* Photodiode_format = "TEMD6200: %f\n";


/* Send a string over USART, where USART is configured for some SERCOM
    sercom: the SERCOM port configured for USART
    str_buf: the string buffer to write

   TODO: This should probably be integrated with Tyler's USART stuff?
*/
void usart_send_string(Sercom* sercom, const char *str_buf) {
  memset(radio_receive_buffer, 0, 16);
  receiveIndex = 0;
  while (*str_buf != '\0') {
    while(!sercom->USART.INTFLAG.bit.DRE);
    sercom->USART.DATA.reg = *str_buf;
    str_buf++;
  }
}



/* Polls the sensors for data, writes to a logging device listening on UART.
    logger: A pointer to a Sercom object i.e. which SERCOM port the logging device
            is connected to. This SERCOM port must already be configured for USART!!
*/
void pollSensors_writeUART(Sercom* logger) {
  char* string_buf[STR_LEN];
  
  /* IR Sensor data: */
  uint16_t rawIRData1 = MLX90614_readRawIRData(MLX90614_DEFAULT_I2CADDR, MLX_IR1);
  uint16_t rawIRData2 = MLX90614_readRawIRData(MLX90614_DEFAULT_I2CADDR, MLX_IR2);

  float ambientTemp = MLX90614_readTempC(MLX90614_DEFAULT_I2CADDR, MLX_TAMB);
  float objTemp = MLX90614_readRawIRData(MLX90614_DEFAULT_I2CADDR, MLX_TOBJ);

  // MLX90614: (RAW_IR1, RAW_IR2), T_AMB, T_OBJ
  sprintf(string_buf, IRSensor_format, rawIRData1, rawIRData2, ambientTemp, objTemp);
  usart_send_string(logger, string_buf);

  /* Temperature sensor data: */
  //TODO

  /* Magnetometer data: */
  uint8_t HMC5883L_data[6];
  HMC5883L_read(HMC5883L_data); // Read the data
  
  int16_t coords_buf[3];
  HMC5883L_getXYZ(HMC5883L_data, coords_buf); // get the x,y,z tuple

  float heading = HMC5883L_computeCompassDir(coords_buf[0], coords_buf[1], coords_buf[2]);

  sprintf(string_buf, Magnetometer_format, coords_buf[0], coords_buf[1], coords_buf[2], heading);
  usart_send_string(logger, string_buf);

  /* Photodiode data: */
  //TODO (i don't understand our ADC API...)

}
