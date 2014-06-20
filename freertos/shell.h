/* Kernel includes. */
#include "FreeRTOS.h"

/* Demo application includes. */
#include "partest.h"
#include "flash.h"

/* Hardware and starter kit includes. */
#include "stm32f4xx.h"

/* The time between cycles of the 'check' functionality (defined within the
tick hook. */
#define mainCHECK_DELAY                     ( ( TickType_t ) 5000 / portTICK_PERIOD_MS )

/* Task priorities. */
#define mainQUEUE_POLL_PRIORITY             ( tskIDLE_PRIORITY + 2 )
#define mainSEM_TEST_PRIORITY               ( tskIDLE_PRIORITY + 1 )
#define mainBLOCK_Q_PRIORITY                ( tskIDLE_PRIORITY + 2 )
#define mainUIP_TASK_PRIORITY               ( tskIDLE_PRIORITY + 3 )
#define mainFLASH_TASK_PRIORITY             ( tskIDLE_PRIORITY + 2 )
#define mainLCD_TASK_PRIORITY               ( tskIDLE_PRIORITY + 3 )
#define mainINTEGER_TASK_PRIORITY           ( tskIDLE_PRIORITY )
#define mainGEN_QUEUE_TASK_PRIORITY         ( tskIDLE_PRIORITY )

/* The WEB server has a larger stack as it utilises stack hungry string
handling library calls. */
#define mainBASIC_WEB_STACK_SIZE            ( configMINIMAL_STACK_SIZE * 4 )

/* The length of the queue used to send messages to the LCD task. */
#define mainQUEUE_SIZE                      ( 3 )

/* The period of the system clock in nano seconds.  This is used to calculate
the jitter time in nano seconds. */
#define mainNS_PER_CLOCK                    ( ( unsigned long ) ( ( 1.0 / ( double ) configCPU_CLOCK_HZ ) * 1000000000.0 ) )

#ifndef SHELL_H
#define SHELL_H

int parse_command(char *str, char *argv[]);

typedef void cmdfunc(int, char *[]);

cmdfunc *do_command(const char *str);

#endif
