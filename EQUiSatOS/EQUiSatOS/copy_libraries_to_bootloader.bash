#!/usr/bin/env bash

# copies library files from main program to bootloader
BOOTLOADER_ROOT="../../bootloader/bootloader"

read -p "This will overwrite the bootloader versions with the EQUiSatOS ones. Continue? "
cp src/processor_drivers/MRAM_Commands.* $BOOTLOADER_ROOT/src/Bootloader/
cp src/processor_drivers/Watchdog_Commands.* $BOOTLOADER_ROOT/src/Bootloader/
