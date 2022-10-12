#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "esp_log.h"

/* FreeRTOS.org includes. */
#include "freertos/FreeRTOSConfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "my_helper_fct.h"

/* Default stack size for tasks */
static const uint32_t STACK_SIZE = 4000;

/* Task Priority */
static const uint32_t T1_PRIO = 5;
static const uint32_t T2_PRIO = 5;
static const uint32_t T3_PRIO = 5;

static const char* TAG= "MsgTimeOut";

/**
 * @brief Starting point function
 * 
 */
QueueHandle_t xQueue1 = NULL;
/* Used as a loop counter to create a very crude delay. */
#define mainDELAY_LOOP_COUNT		(0x1FFFFF)

/* The task function. */
void vTaskFunction1(void *pvParameters);
void vTaskFunction2(void *pvParameters);
void vTaskFunction3(void *pvParameters);


void app_main(void)
{

	xQueue1 = xQueueCreate(5, sizeof(uint32_t *));//depth is 5
	
	printf("Hello ESP32 !\n");
vTaskSuspendAll();
	xTaskCreate(vTaskFunction1,	/* Pointer to the function that implements the task. */
		"Task 1",				/* Text name for the task.  This is to facilitate debugging only. */
		STACK_SIZE,				/* Stack depth  */
		(void*)"Task 1",		/* Pass the text to be printed in as the task parameter. */
		T1_PRIO,				/* Task priority */
		NULL);					/* We are not using the task handle. */

	xTaskCreate(vTaskFunction2,	/* Pointer to the function that implements the task. */
		"Task 2",				/* Text name for the task.  This is to facilitate debugging only. */
		STACK_SIZE,				/* Stack depth  */
		(void*)"Task 2",		/* Pass the text to be printed in as the task parameter. */
		T2_PRIO,				/* Task priority */
		NULL);					/* We are not using the task handle. */
			
	xTaskCreate(vTaskFunction3,	/* Pointer to the function that implements the task. */
		"Task 3",				/* Text name for the task.  This is to facilitate debugging only. */
		STACK_SIZE,				/* Stack depth  */
		(void*)"Task 3",		/* Pass the text to be printed in as the task parameter. */
		T3_PRIO,				/* Task priority */
		NULL);					/* We are not using the task handle. */
xTaskResumeAll();

}


/*-----------------------------------------------------------*/

void vTaskFunction1(void *pvParameters) {
	char *pcTaskName;
	volatile uint32_t ul;
	int32_t value=50;

	/* The string to print out is passed in via the parameter.  Cast this to a
	character pointer. */
	pcTaskName = (char *)pvParameters;
	TickType_t xLastWakeTime = 	xTaskGetTickCount();
	DISPLAY("Start of %s task, priority = %d",pcTaskName, uxTaskPriorityGet(NULL));

	/* As per most tasks, this task is implemented in an infinite loop. */
	for (;; ) {
		DISPLAY("Run computation of %s", pcTaskName);

		/* Delay for simulating a computation */
		for (ul = 0; ul < mainDELAY_LOOP_COUNT; ul++){		
		}

		// Post
		value = 50;
		uint32_t result = xQueueSend (xQueue1, &value, 0) ;
		// Check result
		if ( result == errQUEUE_FULL ){
			DISPLAY("erreur send");
		}
		DISPLAY("value send : %d", value);
		// Compute time : 4 ticks or 40 ms
		COMPUTE_IN_TICK (4) ;
		

	/* Approximated / Periodic Delay */
	
		DISPLAY ( " Periodic Delay of %s " , pcTaskName ) ;
		vTaskDelayUntil (&xLastWakeTime, pdMS_TO_TICKS (500) ) ;
		
		DISPLAY("End of %s", pcTaskName);
	}
}


/*-----------------------------------------------------------*/

void vTaskFunction2(void *pvParameters) {
	char *pcTaskName;
	volatile uint32_t ul;
	int32_t lReceivedValue;
	
	
	//BaseType_t xStatus;
	/* The string to print out is passed in via the parameter.  Cast this to a
	character pointer. */
	pcTaskName = (char *)pvParameters;
	//TickType_t xLastWakeTime = 	xTaskGetTickCount();
	DISPLAY("Start of %s task, priority = %d",pcTaskName, uxTaskPriorityGet(NULL));

	/* As per most tasks, this task is implemented in an infinite loop. */
	/* As per most tasks, this task is implemented in an infinite loop. */
	for (;; ) {
		DISPLAY("Run computation of %s", pcTaskName);

		/* Delay for simulating a computation */
		for (ul = 0; ul < mainDELAY_LOOP_COUNT; ul++){		
		}
		if (xQueueReceive(xQueue1,&lReceivedValue,portMAX_DELAY)){
			DISPLAYI(TAG,"task 2,mess=%d", lReceivedValue);
			COMPUTE_IN_TIME_MS(30);
		}
		else{
			DISPLAYE(TAG,"task 2,timeout !" );
			COMPUTE_IN_TIME_MS(10);
		}

/*		// Post
		 xStatus = xQueueReceive(xQueue1,&lReceivedValue,portMAX_DELAY);
		// Check result
		if ( xStatus == errQUEUE_FULL ){
			DISPLAY("erreur Receive");
		}
		DISPLAY("value received : %d", lReceivedValue);
		// Compute time : 4 ticks or 40 ms
		COMPUTE_IN_TICK (3) ;
*/


		DISPLAY("End of %s", pcTaskName);
	}
}

void vTaskFunction3(void *pvParameters) {
	// Task blocked during 100 ms
	char *pcTaskName;
	volatile uint32_t ul;

	/* The string to print out is passed in via the parameter.  Cast this to a
	character pointer. */
	pcTaskName = (char *)pvParameters;
	//TickType_t xLastWakeTime = 	xTaskGetTickCount();
	DISPLAY("Start of %s task, priority = %d",pcTaskName, uxTaskPriorityGet(NULL));

	vTaskDelay ( pdMS_TO_TICKS (100) ) ;
	for (;; ) {
		DISPLAY("Run computation of %s", pcTaskName);

		/* Delay for simulating a computation */
		for (ul = 0; ul < mainDELAY_LOOP_COUNT; ul++){		
		}

	
		// Compute time : 2 ticks or 20 ms
		COMPUTE_IN_TICK (2) ;
	}

	DISPLAY("End of %s", pcTaskName);
}
