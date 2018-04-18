#!/usr/bin/python

EQUISATOS_CONFIG = "Debug"
BOOTLOADER_CONFIG = "Debug"
EQUISATOS_BIN_LOCATION = "./EQUiSatOS/EQUiSatOS/%s/EQUiSatOS.bin" % EQUISATOS_CONFIG
BOOTLOADER_BIN_LOCATION = "./bootloader/bootloader/%s/bootloader.bin" % BOOTLOADER_CONFIG
COMBINED_BIN_LOCATION = "combined.bin"
NOOP_INSTR_BYTES = chr(0x0000)
EQUISATOS_START_ADDR = 0x6000

with open(BOOTLOADER_BIN_LOCATION, "r") as bootloader:
    bootloader_data = bootloader.read()
    bootloader_size = len(bootloader_data)

    # check that it fits
    assert bootloader_size < EQUISATOS_START_ADDR, \
        "Bootloader size exceeds start address by %d! (%d vs. %d)" % \
        (bootloader_size-EQUISATOS_START_ADDR, bootloader_size, EQUISATOS_START_ADDR)

    print("Opened bootloader @ %s (size=%d)" % (BOOTLOADER_BIN_LOCATION, bootloader_size))

    with open(EQUISATOS_BIN_LOCATION, "r") as equisatos:
        equisatos_data = equisatos.read()
        equisatos_size = len(equisatos_data)
        print("Opened EQUiSatOS @ %s (size=%d)" % (EQUISATOS_BIN_LOCATION, equisatos_size))

        # we should have some space between the end of the bootloader
        # and the binary, so fill that space with no-op instructions
        # that will slide any instruction executions at those addresses
        # up to the app start address
        noop_str_length = EQUISATOS_START_ADDR - bootloader_size
        noop_str_data = NOOP_INSTR_BYTES * noop_str_length
        assert len(noop_str_data) == noop_str_length, \
            "expected noop len: %d, real: %d" % (noop_str_length, len(noop_str_data))
        print("Adding %d bytes of NOOP instructions between bootloader & EQUiSatOS" % noop_str_length)

        # insert bootloader at start of binary,
        # then add NOOP instructions up to the start of the EQUiSatOS,
        # and then add the EQUiSatOS binary
        with open(COMBINED_BIN_LOCATION, "w") as combined:
            combined.write(bootloader_data)
            assert combined.tell() == bootloader_size, \
                "combined size: %d, expected size: %d" % (combined.tell(), bootloader_size)
            combined.write(noop_str_data)
            assert combined.tell() == EQUISATOS_START_ADDR, \
                "combined size: %d, expected size: %d" % (combined.tell(), EQUISATOS_START_ADDR)
            combined.write(equisatos_data)
            assert combined.tell() == EQUISATOS_START_ADDR + equisatos_size, \
                "combined size: %d, expected size: %d" % (combined.tell(), EQUISATOS_START_ADDR + equisatos_size)

        print("Wrote to combined file @ %s" % COMBINED_BIN_LOCATION)
