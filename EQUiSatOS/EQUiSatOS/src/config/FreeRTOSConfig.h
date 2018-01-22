#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/* For documentation for all the configuration symbols, go to:
 * http://www.freertos.org/a00110.html.
 */

#if defined (__GNUC__) || defined (__ICCARM__)
/* Important: put #includes here unless they are also meant for the assembler.
 */
#include <stdint.h>
void assert_triggered( const char * file, uint32_t line );
#endif

#define configUSE_PREEMPTION                    1
#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0
#define configPRIO_BITS                         2
#define configCPU_CLOCK_HZ                      ( 8000000 ) // (system_gclk_gen_get_hz(GCLK_GENERATOR_0))
#define configTICK_RATE_HZ                      ( ( TickType_t ) 1000 ) // so portTICK_PERIOD_MS = 1
#define configMAX_PRIORITIES                    ( ( uint32_t ) 5 )
#define configMINIMAL_STACK_SIZE                ( ( uint16_t ) 100 )
/* configTOTAL_HEAP_SIZE is not used when heap_3.c is used. */
#define configTOTAL_HEAP_SIZE                   ( ( size_t ) ( 0 ) )
#define configSUPPORT_STATIC_ALLOCATION         1
#define configSUPPORT_DYNAMIC_ALLOCATION		0
#define configAPPLICATION_ALLOCATED_HEAP		0 // no need if no heap
#define configMAX_TASK_NAME_LEN                 ( 8 )
#define configUSE_TRACE_FACILITY                0			// SET to 1 to use Tracelyzer; 0 to free up the RAM space
#define configUSE_16_BIT_TICKS                  0
#define configIDLE_SHOULD_YIELD                 1
#define configUSE_MUTEXES                       1
#define configQUEUE_REGISTRY_SIZE               0
#define configCHECK_FOR_STACK_OVERFLOW          2 // TODO: may want to disable / reduce on sat (slowish; see here: https://www.freertos.org/Stacks-and-stack-overflow-checking.html)
#define configUSE_RECURSIVE_MUTEXES             0 
#define configUSE_MALLOC_FAILED_HOOK            0
#define configUSE_COUNTING_SEMAPHORES           1
#define configUSE_QUEUE_SETS                    0
#define configUSE_TICKLESS_IDLE					0
#define configGENERATE_RUN_TIME_STATS           0
#define configENABLE_BACKWARD_COMPATIBILITY     0 // for Tracelyzer streaming mode

/* Co-routine definitions. */
#define configUSE_CO_ROUTINES                   0
#define configMAX_CO_ROUTINE_PRIORITIES         ( 2 )

/* Software timer definitions. */
#define configUSE_TIMERS                        1
#define configTIMER_TASK_PRIORITY               ( 2 )
#define configTIMER_QUEUE_LENGTH                2
#define configTIMER_TASK_STACK_DEPTH            ( 80 )

/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */
#define INCLUDE_vTaskPrioritySet                1
#define INCLUDE_uxTaskPriorityGet               1
#define INCLUDE_vTaskDelete                     1
#define INCLUDE_vTaskSuspend                    1
#define INCLUDE_xReameFromISR                  1
#define INCLUDE_vTaskDelayUntil                 1
#define INCLUDE_vTaskDelay                      1
#define INCLUDE_xTaskGetSchedulerState          1
#define INCLUDE_xTaskGetCurrentTaskHandle       1
#define INCLUDE_uxTaskGetStackHighWaterMark     0
#define INCLUDE_xTaskGetIdleTaskHandle          0
#define INCLUDE_xTimerGetTimerDaemonTaskHandle  0
#define INCLUDE_pcTaskGetTaskName               0
#define INCLUDE_eTaskGetState                   1


/* Normal assert() semantics without relying on the provision of an assert.h
header file. */
#define configASSERT( x ) \
        if( ( x ) == 0 ) { taskDISABLE_INTERRUPTS(); for( ;; ); }

/* Definitions that map the FreeRTOS port interrupt handlers to their CMSIS
standard names - or at least those used in the unmodified vector table. */
#define vPortSVCHandler                         SVC_Handler
#define xPortPendSVHandler                      PendSV_Handler
#define xPortSysTickHandler                     SysTick_Handler

/* The lowest (highest-valued) interrupt priority that can be used in a call to 
a "set priority" function. */
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY			0x3

/* The highest interrupt priority that can be used by any interrupt service
routine that makes calls to interrupt safe FreeRTOS API functions.  DO NOT CALL
INTERRUPT SAFE FREERTOS API FUNCTIONS FROM ANY INTERRUPT THAT HAS A HIGHER
PRIORITY THAN THIS! (higher priorities are lower numeric values.) */
// err heavily on the side of safety (over speed) by allowing any interrupt 
// to use FreeRTOS API functions and therefore not blocking off any interrupts
// in RTOS critical sections (this helps prevent associated hard faults)
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY	0x0

/* Interrupt priorities used by the kernel port layer itself.  These are generic
to all Cortex-M ports, and do not rely on any particular library functions. */
#define CORTEX_M_ISR_PRIORITY_MASK				(0xff >> configPRIO_BITS)
// (the intent is to set the top configPRIO_BITS to the values, and the rest of the bits to 1)
#define configKERNEL_INTERRUPT_PRIORITY 		( CORTEX_M_ISR_PRIORITY_MASK | (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS)) )
#define configMAX_SYSCALL_INTERRUPT_PRIORITY 	( CORTEX_M_ISR_PRIORITY_MASK | (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS)) )

/* Integrates the Tracealyzer recorder with FreeRTOS */
#if ( configUSE_TRACE_FACILITY == 1 )
	#include "trcRecorder.h"
#endif

#endif /* FREERTOS_CONFIG_H */
