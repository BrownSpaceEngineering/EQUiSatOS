# CubesatFreeRTOS

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

