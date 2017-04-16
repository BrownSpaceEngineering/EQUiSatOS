<<<<<<< acdfa3af522a93d0f556a3465cfc0ad35a0e5f2e
################################################################################
# Automatically-generated file. Do not edit or delete the file
################################################################################

src\testing_functions\Num_Stack_Test.c

src\runnable_configurations\isItOn.c

src\testing_functions\watchdog_task_test.c

src\ASF\common\utils\interrupt\interrupt_sam_nvic.c

src\ASF\sam0\boards\samd21_xplained_pro\board_init.c

src\ASF\sam0\drivers\adc\adc_sam_d_r\adc.c

src\ASF\sam0\drivers\rtc\rtc_sam_d_r\rtc_calendar.c

src\ASF\sam0\drivers\rtc\rtc_sam_d_r\rtc_calendar_interrupt.c

src\ASF\sam0\drivers\sercom\i2c\i2c_sam0\i2c_master.c

src\ASF\sam0\drivers\system\clock\clock_samd21_r21_da\clock.c

src\ASF\sam0\drivers\system\clock\clock_samd21_r21_da\gclk.c

src\ASF\sam0\drivers\system\interrupt\system_interrupt.c

src\ASF\sam0\drivers\system\pinmux\pinmux.c

src\ASF\sam0\drivers\system\system.c

src\ASF\sam0\drivers\tcc\tcc.c

src\ASF\sam0\drivers\wdt\wdt.c

src\ASF\sam0\drivers\wdt\wdt_callback.c

src\ASF\sam0\utils\cmsis\samd21\source\gcc\startup_samd21.c

src\ASF\sam0\utils\cmsis\samd21\source\system_samd21.c

src\ASF\sam0\utils\syscalls\gcc\syscalls.c

src\main.c

src\ASF\sam0\drivers\sercom\spi\spi.c

src\ASF\sam0\utils\stdio\read.c

src\ASF\sam0\utils\stdio\write.c

src\ASF\thirdparty\freertos\freertos-8.0.1\Source\croutine.c

src\ASF\thirdparty\freertos\freertos-8.0.1\Source\event_groups.c

src\ASF\thirdparty\freertos\freertos-8.0.1\Source\portable\GCC\ARM_CM0\port.c

src\ASF\thirdparty\freertos\freertos-8.0.1\Source\portable\MemMang\heap_1.c

src\ASF\thirdparty\freertos\freertos-8.0.1\Source\list.c

src\ASF\thirdparty\freertos\freertos-8.0.1\Source\queue.c

src\ASF\thirdparty\freertos\freertos-8.0.1\Source\tasks.c

src\ASF\thirdparty\freertos\freertos-8.0.1\Source\timers.c

src\ASF\sam0\drivers\sercom\usart\usart.c

src\ASF\sam0\drivers\sercom\sercom.c

src\ASF\common2\services\delay\sam0\systick_counter.c

src\ASF\common\utils\unit_test\suite.c

src\ASF\sam0\drivers\nvm\nvm.c

src\ASF\sam0\drivers\port\port.c

src\processor_drivers\ADC_Commands.c

src\processor_drivers\Direct_Pin_Commands.c

src\processor_drivers\I2C_Commands.c

src\processor_drivers\PWM_Commands.c

src\processor_drivers\RTC_Commands.c

src\processor_drivers\SPI_Commands.c

src\processor_drivers\USART_Commands.c

src\processor_drivers\Watchdog_Commands.c

src\processor_drivers\Watchdog_Task.c

src\radio\Radio_Commands.c

src\runnable_configurations\flatsat.c

src\radio\Stacks\boot_Stack.c

src\radio\Stacks\flash_Stack.c

src\radio\Stacks\idle_Stack.c

src\radio\Stacks\low_power_Stack.c

src\radio\Stacks\Num_Stack.c

src\runnable_configurations\init_rtos_tasks.c

src\runnable_configurations\rtos_tasks.c

src\runnable_configurations\scratch_testing.c

src\sensor_drivers\AD7991_ADC.c

src\runnable_configurations\Sensor_Structs.c

src\sensor_drivers\LTC1380_Multiplexer_Commands.c

src\runnable_configurations\Sensor_Structs.c

src\sensor_drivers\MLX90614_IR_Sensor.c

src\sensor_drivers\MPU9250_9axis_Commands.c

src\sensor_drivers\sensor_read_commands.c

src\sensor_drivers\switching_commands.c

src\sensor_drivers\TCA9535_GPIO.c

src\sensor_drivers\TEMD6200_Commands.c

=======
################################################################################
# Automatically-generated file. Do not edit or delete the file
################################################################################

src\ASF\sam0\drivers\adc\adc_sam_d_r\adc_callback.c

<<<<<<< 41baf7d09593025b1c7293477c977e5ec22effca
<<<<<<< 1a8ab1dd96a07475594f52d35aebd85603a331b5
<<<<<<< 462bf27d0672cf75135b22b37336eeed3798a9f8
<<<<<<< 307ec729d99c6bb04a01794d9132ba9106009929
=======
src\radio\Stacks\Data_Heap.c
=======
src\ASF\thirdparty\freertos\freertos-8.0.1\Source\portable\MemMang\heap_4.c
>>>>>>> Stylistic adjustments
=======
src\ASF\thirdparty\freertos\freertos-8.0.1\Source\portable\MemMang\heap_1.c
>>>>>>> Convert to using heap_1.c; stencil for transmission code

src\runnable_configurations\rtos_task_frequencies.c

src\runnable_configurations\struct_tests.c

src\runnable_configurations\task_testing.c

<<<<<<< 7217e1a9d836e30b58524d54454366ff41c7fde5
<<<<<<< 09ea68c79a4b3a9e249dd6f6919dae03fa1173fc
src\stacks\boot_Stack.c

<<<<<<< a80674390dc307fccd74b9c174b0b9c2490d0573
<<<<<<< 1a8ab1dd96a07475594f52d35aebd85603a331b5
>>>>>>> Implemented basic malloc/free for data structs; removed timestamps from batches
src\ASF\thirdparty\freertos\freertos-8.0.1\Source\portable\MemMang\heap_4.c
=======
src\radio\Stacks\Num_Stack.c
>>>>>>> Fixed mutexes and updated num_stack
=======
src\stacks\data_heap.c

src\stacks\data_heap_testing.c
>>>>>>> Stylistic adjustments
=======
=======
>>>>>>> Merge idle and low power
=======
src\stacks\equistack.c

<<<<<<< db42bed40dee83dbe7ae3a1c86f94d5368f0a0b6
>>>>>>> Create generic equistack (will it work?)
src\stacks\equistacks.c
>>>>>>> Merge data_t_heap and equistacks into "staged" equistacks
=======
src\stacks\package_transmission.c
>>>>>>> more tests (working now) and deleted unused files

src\testing_functions\test_data.c

src\testing_functions\test_stacks.c

<<<<<<< db42bed40dee83dbe7ae3a1c86f94d5368f0a0b6
<<<<<<< 1a8ab1dd96a07475594f52d35aebd85603a331b5
src\radio\Stacks\stack_tests.c

src\radio\Stacks\State_Structs.c
=======
src\stacks\idle_Stack.c
>>>>>>> Stylistic adjustments

src\stacks\Num_Stack.c

src\stacks\package_stack.c

<<<<<<< a494f1d5f3b0c3a293acdb42391f90442f899246
src\stacks\Sensor_Structs.c

src\stacks\State_Structs.c

<<<<<<< 8cbb62e29bf7f8bdd28e8369fee79d69ae0976ad
src\stacks\State_Structs.c

=======
=======
>>>>>>> Refactored basically everything
src\stacks\test_data.c

src\stacks\test_stacks.c

src\stacks\test_structs.c
=======
src\testing_functions\test_structs.c
>>>>>>> more tests (working now) and deleted unused files

>>>>>>> Added v basic tests and found a bug in Equistack
src\ASF\common\utils\interrupt\interrupt_sam_nvic.c

src\ASF\sam0\boards\samd21_xplained_pro\board_init.c

src\ASF\sam0\drivers\adc\adc_sam_d_r\adc.c

src\ASF\sam0\drivers\rtc\rtc_sam_d_r\rtc_calendar.c

src\ASF\sam0\drivers\rtc\rtc_sam_d_r\rtc_calendar_interrupt.c

src\ASF\sam0\drivers\sercom\i2c\i2c_sam0\i2c_master.c

src\ASF\sam0\drivers\system\clock\clock_samd21_r21_da\clock.c

src\ASF\sam0\drivers\system\clock\clock_samd21_r21_da\gclk.c

src\ASF\sam0\drivers\system\interrupt\system_interrupt.c

src\ASF\sam0\drivers\system\pinmux\pinmux.c

src\ASF\sam0\drivers\system\system.c

src\ASF\sam0\utils\cmsis\samd21\source\gcc\startup_samd21.c

src\ASF\sam0\utils\cmsis\samd21\source\system_samd21.c

src\ASF\sam0\utils\syscalls\gcc\syscalls.c

src\main.c

src\ASF\sam0\drivers\sercom\spi\spi.c

src\ASF\sam0\utils\stdio\read.c

src\ASF\sam0\utils\stdio\write.c

src\ASF\thirdparty\freertos\freertos-8.0.1\Source\croutine.c

src\ASF\thirdparty\freertos\freertos-8.0.1\Source\event_groups.c

src\ASF\thirdparty\freertos\freertos-8.0.1\Source\portable\GCC\ARM_CM0\port.c

src\ASF\thirdparty\freertos\freertos-8.0.1\Source\list.c

src\ASF\thirdparty\freertos\freertos-8.0.1\Source\queue.c

src\ASF\thirdparty\freertos\freertos-8.0.1\Source\tasks.c

src\ASF\thirdparty\freertos\freertos-8.0.1\Source\timers.c

src\ASF\sam0\drivers\sercom\usart\usart.c

src\ASF\sam0\drivers\sercom\sercom.c

src\ASF\common2\services\delay\sam0\systick_counter.c

src\ASF\common\utils\unit_test\suite.c

src\ASF\sam0\drivers\nvm\nvm.c

src\ASF\sam0\drivers\port\port.c

src\processor_drivers\ADC_Commands.c

src\processor_drivers\Direct_Pin_Commands.c

src\processor_drivers\I2C_Commands.c

src\processor_drivers\RTC_Commands.c

src\processor_drivers\SPI_Commands.c

src\processor_drivers\USART_Commands.c

src\radio\Radio_Commands.c

src\runnable_configurations\init_rtos_tasks.c

src\runnable_configurations\rtos_tasks.c

src\runnable_configurations\scratch_testing.c

src\sensor_drivers\HMC5883L_Magnetometer_Commands.c

src\sensor_drivers\MLX90614_IR_Sensor.c

src\sensor_drivers\sensor_read_commands.c

src\sensor_drivers\switching_commands.c

src\sensor_drivers\TEMD6200_Commands.c

>>>>>>> Wrote tests for free, made incremental updates to other files
