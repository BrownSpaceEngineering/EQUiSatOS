Changing the starting address in Atmel studio

For the bootloader it is just necessary to change APP_START_ADDRESS to the chosen  address and use the device programmer to upload the binary.

Applications loaded from the mram to flash will not allow debugging. However, you can load an application from a particular address while using the bootloader.
So, first you have to load the bootloader, as explained before.
Then, in the solution go in properties:

Tool - set programming settings to erase only programming area.
Toolchain -> Linker -> include the string: -Wl,-section-start=bootloader=0x3000 -Wl,
It will make the program start at the address 0x3000, for example.