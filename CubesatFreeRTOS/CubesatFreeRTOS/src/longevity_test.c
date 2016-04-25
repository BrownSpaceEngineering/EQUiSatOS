/* Longevity testing involves polling sensors for data, and writing output
   to the OpenLog (via SERCOM)

   Author: Arun
   Date: 24.04.2016
*/

#include "main.h"

/* Sensors to poll:
     - IR sensor
        * Raw IR channels 1 and 2
        * Ambient temperature
        * Object temperature
     - Temperature sensor
        * Ambient temperature
     - Magnetometer
        * Read magnetic field strengths in x,y,z directions
        * Get compass heading (from xyz)
*/

// TODO: Everything
