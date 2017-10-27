# EQUiSatOS

## Project

We are a student-run engineering group at Brown University. This project runs on our satellite, EQUiSat, which will launch in March 2018.
It uses an Atmel SAMD21J18A chip and Atmel's developer tools. We are open source, bringing space to the people!
Contact: [Website](http://brownspace.org/) and [e-mail](mailto:bse@brown.edu)

## Documentation

A lot of documentation should exist on the Google Drive. Ideally, for CubeSat driver
API documentation, check out the respective C files for each device - they should
hopefully be well-commented.

## Building

### Building on Windows

1. Install Atmel Studio from Atmel's website.
2. Open the CubesatFreeRTOS.atsln file from the parent directory
3. Build the project in Atmel Studio

### Building on Unix-like Systems (Linux, OS X)

1. Acquire the `arm-none-eabi-binutils`, `arm-none-eabi-gcc`, `arm-none-eabi-newlib` packages. These provide the standard binutils (ld, objdump, objcopy, etc) along with compiler (gcc) and standard C library files.
2. Add the executables from the above step to your PATH. If you installed via package manager, you should be good to go.
3. Use the `./makelinux` script to convert the Atmel Studio Makefile to a Unix-compatible version and build the executables.

## Flashing

### Flashing on Windows

1. Can run "Debug" from Atmel Studio, or can upload via Atmel Studio

### Flashing on Linux/OS X

This is a work in progress. Currently, openOCD seems to be the most viable solution.
There is a config file for the SAM DXX boards (boards which use an ARM Cortex-M0+
processor) which may be what we need. Tentatively, the following *may* work:
```
  openocd -f target/at91samdXX.cfg
```
Then, in a separate terminal:
```
  telnet localhost 4444
```
Run the following within telnet:
```
  reset halt
  flash write_image erase <path to CubesatFreeRTOS.hex>
  reset run
```

