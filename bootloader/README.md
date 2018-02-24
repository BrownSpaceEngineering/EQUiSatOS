## How to load the bootloader and binary onto the processor flash (for _writing_ to MRAM)

Applications loaded from the mram to flash will not allow debugging. However, you can load an application from a particular address while using the bootloader (this what the bootloader normally does).
So, first you have to write the binary you want to store in the MRAM into program flash (so it can be accessed), and then load the bootloader without overwriting that binary.

### In the project of the binary you want to load
Toolchain -> Linker -> Memory Settings: add a FLASH segment as `.text=<app_start_address>`
- This ensures all the addresses in the code are correctly offset when put in the new address space
- It will also make the program start at that address (0x3000, for example).
- MAKE SURE TO REBUILD or the ELF won't have the offset properly configured
Check "Override Vector Table Offset Register" and enter your `<app_start_address>`

### In the bootloader project
*In the code:*
Change the APP_START_ADDRESS to the chosen address and use the device programmer to upload the binary.

*In Tools->Device Programming:*
- Configure the connected device to access its settings
- Go to 'memories'
- Under Flash, navigate to the .bin file that is placed in the build folder of your main project. (ex. Debug/EQUiSatOS.bin) (you'll need to change the filetypes in the dialog to see it)
- The "erase flash before programming" and "verify flash after programming" checkboxes don't matter; you can leave them checked
- Under Advanced, set the programming address to the APP_START_ADDRESS you decided on (this is why we did the binary)

*In the solution properties:*
Tool - set programming settings to erase only programming area
- This way, when the bootloader is written, it won't overwrite the binary we wrote to APP_START_ADDRESS
