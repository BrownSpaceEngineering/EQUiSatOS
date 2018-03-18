#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/* For documentation for all the configuration symbols, go to:
 * http://www.freertos.org/a00110.html.
 */

#if defined (__GNUC__) || defined (__ICCARM__)
/* Important: put #includes here unless they are also meant for the assembler.
 */
#include <stdint.h>
#include "config.h"
void assert_triggered( const char * file, uint32_t line );
#endif

#define configUSE_PREEMPTION                    1
#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0
#define configPRIO_BITS                         2
#define configCPU_CLOCK_HZ                      ( 8000000 ) // (system_gclk_gen_get_hz(GCLK_GENERATOR_0))
#define configTICK_RATE_HZ                      ( ( TickType_t ) 1000 ) // so portTICK_PERIOD_MS = 1
#define configMAX_PRIORITIES                    ( ( uint32_t ) 6 )
#define configMINIMAL_STACK_SIZE                ( ( uint16_t ) 100 )
/* configTOTAL_HEAP_SIZE is not used when heap_3.c is used. */
#define configTOTAL_HEAP_SIZE                   ( ( size_t ) ( 0 ) )
#define configSUPPORT_STATIC_ALLOCATION         1
#define configSUPPORT_DYNAMIC_ALLOCATION		0
#define configAPPLICATION_ALLOCATED_HEAP		0 // no need if no heap
#define configMAX_TASK_NAME_LEN                 ( 8 )
#define configUSE_TRACE_FACILITY                USE_TRACELYZER		// SET to 1 to use Tracelyzer; 0 to free up the RAM space
#define configUSE_16_BIT_TICKS                  0
#define configIDLE_SHOULD_YIELD                 1
#define configUSE_MUTEXES                       1
#define configQUEUE_REGISTRY_SIZE               0
#define configCHECK_FOR_STACK_OVERFLOW			defined(RELEASE) ? 1 : 2 // disable / reduce on sat (slowish; see here: https://www.freertos.org/Stacks-and-stack-overflow-checking.html)
#define configUSE_RECURSIVE_MUTEXES             (PRINT_DEBUG == 1 || PRINT_DEBUG == 3)
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
#define INCLUDE_vTaskPrioritySet                0
#define INCLUDE_uxTaskPriorityGet               0
#define INCLUDE_vTaskDelete                     1
#define INCLUDE_vTaskSuspend                    1
#define INCLUDE_xReameFromISR                   1
#define INCLUDE_vTaskDelayUntil                 1
#define INCLUDE_vTaskDelay                      1
#define INCLUDE_xTaskAbortDelay					1
#define INCLUDE_xTaskGetSchedulerState          1
#define INCLUDE_xTaskGetCurrentTaskHandle       1
#define INCLUDE_uxTaskGetStackHighWaterMark     1
#define INCLUDE_xTaskGetIdleTaskHandle          0
#define INCLUDE_xTimerGetTimerDaemonTaskHandle  0
#define INCLUDE_pcTaskGetTaskName               0
#define INCLUDE_eTaskGetState                   1


/* Normal assert() semantics without relying on the provision of an assert.h
header file. */
#ifdef USE_ASSERTIONS
	#define configASSERT( x ) \
			 if( ( x ) == 0 ) { taskDISABLE_INTERRUPTS(); for( ;; ); }
#else
	#define configASSERT(x) do {} while(0);
#endif

/* Definitions that map the FreeRTOS port interrupt handlers to their CMSIS
standard names - or at least those used in the unmodified vector table. */
#define vPortSVCHandler                         SVC_Handler
#define xPortPendSVHandler                      PendSV_Handler
#define xPortSysTickHandler                     SysTick_Handler

// Note that although https://www.freertos.org/a00110.html#kernel_priority suggests 
// that we need to define configKERNEL_INTERRUPT_PRIORITY and configMAX_SYSCALL_INTERRUPT_PRIORITY,
// we DO NOT need to, because we're running the Cortex-M0+, not the Cortex-M3's and M4's that article
// is describing. See here for more the most details (see SUMMARY at end):
// https://mcuoneclipse.com/2016/08/28/arm-cortex-m-interrupts-and-freertos-part-3/ 
// To reassure yourself, look at ASF\thirdparty\freertos\freertos-9.0.0\Source\portable\GCC\ARM_CM0\port.c
// (lines 88-89; note how the interrupt priority is NOT user-configurable but is rather a property of the port)

/* Integrates the Tracealyzer recorder with FreeRTOS */
#if ( configUSE_TRACE_FACILITY == 1 )
	#include "trcRecorder.h"
#endif

#endif /* FREERTOS_CONFIG_H */
