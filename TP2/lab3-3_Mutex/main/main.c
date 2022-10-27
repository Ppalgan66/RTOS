/****************************************************************************
 * Copyright (C) 2020 by Fabrice Muller                                     *
 *                                                                          *
 * This file is useful for ESP32 Design course.                             *
 *                                                                          *
 ****************************************************************************/

/**
 * @file lab3-1_main.c
 * @author Fabrice Muller
 * @date 20 Oct. 2020
 * @brief File containing the lab3-1 of Part 3.
 *
 * @see https://github.com/fmuller-pns/esp32-vscode-project-template
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include "esp_log.h"

/* FreeRTOS.org includes. */
#include "freertos/FreeRTOSConfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "my_helper_fct.h"

static const char* TAG = "SEM";

/* Application constants */
#define STACK_SIZE     4096
#define TABLE_SIZE     400


/* Communications */
#define mainDELAY_LOOP_COUNT (0x1FFFFF)
#define PINNED_TO_CORE 5

/* Task Priority */
const uint32_t TIMER_TASK_PRIORITY = 5;
const uint32_t TIMER_TASK_CORE = 0;
const uint32_t INSPECTOR_TASK_PRIORITY = 4;
const uint32_t INSPECTOR_TASK_CORE = 1;

#if PINNED_TO_CORE == 1
	const uint32_t INC_TABLE_TASK_PRIORITY = 4;
	const uint32_t DEC_TABLE_TASK_PRIORITY = 4;
	const uint32_t INC_TABLE_TASK_CORE = 0;
	const uint32_t DEC_TABLE_TASK_CORE = 0;
#elif PINNED_TO_CORE == 2
	const uint32_t INC_TABLE_TASK_PRIORITY = 3;
	const uint32_t DEC_TABLE_TASK_PRIORITY = 4;
	const uint32_t INC_TABLE_TASK_CORE = 0;
	const uint32_t DEC_TABLE_TASK_CORE = 0;
#elif PINNED_TO_CORE == 3
	const uint32_t INC_TABLE_TASK_PRIORITY = 4;
	const uint32_t DEC_TABLE_TASK_PRIORITY = 4;	
	const uint32_t INC_TABLE_TASK_CORE = 0;
	const uint32_t DEC_TABLE_TASK_CORE = 1;
#elif PINNED_TO_CORE == 4
	const uint32_t INC_TABLE_TASK_PRIORITY = 3;
	const uint32_t DEC_TABLE_TASK_PRIORITY = 4;
	const uint32_t INC_TABLE_TASK_CORE = 0;
	const uint32_t DEC_TABLE_TASK_CORE = 1;
#elif PINNED_TO_CORE == 5
	const uint32_t INC_TABLE_TASK_PRIORITY = 3;
	const uint32_t DEC_TABLE_TASK_PRIORITY = 3;
	const uint32_t INC_TABLE_TASK_CORE = 0;
	const uint32_t DEC_TABLE_TASK_CORE = 0;
#endif
/* Tasks */
void vTaskTimer(void *pvParameters);
void vTaskIncTable(void *pvParameters);
void vTaskDecTable(void *pvParameters);
void vTaskInspector(void *pvParameters);

SemaphoreHandle_t xSemIncClk;
SemaphoreHandle_t xSemDecClk;
SemaphoreHandle_t xSemMutex;


/* Datas */
int Table[TABLE_SIZE];
int constNumber = 1;

/* Main function */
void app_main(void) {

	/* Init Table */
	memset(Table, 0, TABLE_SIZE*sizeof(int));
	for(int index=0;index<TABLE_SIZE;index++){
		Table[index]=index;
	}
	xSemIncClk = xSemaphoreCreateBinary() ;
	xSemDecClk = xSemaphoreCreateBinary() ;
	xSemMutex = xSemaphoreCreateMutex() ;
	
		
	/* Stop scheduler */
	vTaskSuspendAll();

	/* Create Tasks */
	//printf("Hello ESP32 !\n"); pas le droit de mettre 

	xTaskCreatePinnedToCore(vTaskTimer,	/* Pointer to the function that implements the task. */
		"vTaskTimer",				/* Text name for the task.  This is to facilitate debugging only. */
		STACK_SIZE,				/* Stack depth  */
		(void*)"vTaskTimer",		/* Pass the text to be printed in as the task parameter. */
		TIMER_TASK_PRIORITY,				/* Task priority */
		NULL, TIMER_TASK_CORE);					/* We are not using the task handle. */

		

	xTaskCreatePinnedToCore(vTaskIncTable,	/* Pointer to the function that implements the task. */
		"vTaskIncTable",				/* Text name for the task.  This is to facilitate debugging only. */
		STACK_SIZE,				/* Stack depth  */
		(void*)"vTaskIncTable",		/* Pass the text to be printed in as the task parameter. */
		 INC_TABLE_TASK_PRIORITY ,				/* Task priority */
		NULL,INC_TABLE_TASK_CORE);					/* We are not using the task handle. */
			
	xTaskCreatePinnedToCore(vTaskDecTable,	/* Pointer to the function that implements the task. */
		"vTaskDecTable",				/* Text name for the task.  This is to facilitate debugging only. */
		STACK_SIZE,				/* Stack depth  */
		(void*)"vTaskDecTable",		/* Pass the text to be printed in as the task parameter. */
		DEC_TABLE_TASK_PRIORITY,				/* Task priority */
		NULL,DEC_TABLE_TASK_CORE);					/* We are not using the task handle. */

	xTaskCreatePinnedToCore(vTaskInspector,	/* Pointer to the function that implements the task. */
		"vTaskInspector",				/* Text name for the task.  This is to facilitate debugging only. */
		STACK_SIZE,				/* Stack depth  */
		(void*)"vTaskInspector",		/* Pass the text to be printed in as the task parameter. */
		INSPECTOR_TASK_PRIORITY,				/* Task priority */
		NULL,INSPECTOR_TASK_CORE);					/* We are not using the task handle. */



	/* Continue scheduler */
	xTaskResumeAll();

	/* to ensure its exit is clean */
	vTaskDelete(NULL);
}
/*-----------------------------------------------------------*/

void vTaskTimer(void *pvParameters) {

	char *pcTaskName;
	volatile uint32_t ul;

	/* The string to print out is passed in via the parameter.  Cast this to a
	character pointer. */
	pcTaskName = (char *)pvParameters;
	TickType_t xLastWakeTime = 	xTaskGetTickCount();
	DISPLAY("Start of %s task, priority = %d",pcTaskName, uxTaskPriorityGet(NULL));

	/* As per most tasks, this task is implemented in an infinite loop. */
	for (;; ) {
		vTaskDelayUntil (&xLastWakeTime, pdMS_TO_TICKS (250) ) ;
		COMPUTE_IN_TICK (2) ;
		DISPLAY("Task Timer : give sem");
		xSemaphoreGive ( xSemIncClk ) ;
		xSemaphoreGive ( xSemDecClk ) ;
	}
}

void vTaskIncTable(void *pvParameters) {
	char *pcTaskName;
	uint32_t ActivationNumber=0;
	/* The string to print out is passed in via the parameter.  Cast this to a
	character pointer. */
	pcTaskName = (char *)pvParameters;
	TickType_t xLastWakeTime = 	xTaskGetTickCount();
	DISPLAY("Start of %s task, priority = %d",pcTaskName, uxTaskPriorityGet(NULL));

	/* As per most tasks, this task is implemented in an infinite loop. */
	for (;; ) {
		if (xSemaphoreTake ( xSemIncClk , portMAX_DELAY )){
			if(xSemaphoreTake ( xSemMutex , portMAX_DELAY )){
				DISPLAY("Run computation of %s", pcTaskName);
				if(ActivationNumber==0){
					for(int index=0;index<TABLE_SIZE; index++ ){
						Table[index]= Table[index] + constNumber;
					}
					COMPUTE_IN_TICK (5) ;
				ActivationNumber = 4;
				}else {
					ActivationNumber= ActivationNumber-1;
				}
				xSemaphoreGive ( xSemMutex ) ;
				
			} 
		}
	
	}
}

void vTaskDecTable(void *pvParameters) {
	char *pcTaskName;
	
	/* The string to print out is passed in via the parameter.  Cast this to a
	character pointer. */
	pcTaskName = (char *)pvParameters;
	TickType_t xLastWakeTime = 	xTaskGetTickCount();
	DISPLAY("Start of %s task, priority = %d",pcTaskName, uxTaskPriorityGet(NULL));

	/* As per most tasks, this task is implemented in an infinite loop. */
	for (;; ) {
		if (xSemaphoreTake ( xSemDecClk , portMAX_DELAY )){
			
			if (xSemaphoreTake ( xSemMutex , portMAX_DELAY )){
				DISPLAY("Run computation of %s", pcTaskName);
		
				for(int index=0;index<TABLE_SIZE; index++ ){
					Table[index]= Table[index] - 1;
				}
				COMPUTE_IN_TICK (5) ;	
			}
				
			xSemaphoreGive ( xSemMutex );
		}
	
	}
}
void vTaskInspector(void *pvParameters) {
	char *pcTaskName;
	
	/* The string to print out is passed in via the parameter.  Cast this to a
	character pointer. */
	pcTaskName = (char *)pvParameters;
	TickType_t xLastWakeTime = 	xTaskGetTickCount();
	DISPLAY("Start of %s task, priority = %d",pcTaskName, uxTaskPriorityGet(NULL));

	/* As per most tasks, this task is implemented in an infinite loop. */
	for (;; ) {
		xSemaphoreTake ( xSemMutex , portMAX_DELAY ) ;
		DISPLAY("task inspector is checking.");
			
		int reference = Table[0];
		bool error = false;
				
		for(int index=1;index<TABLE_SIZE ;index++ ){
			COMPUTE_IN_TIME_US (100) ;
			if (Table[index] != reference+index){
				error = true;
			}
		}
		DISPLAY("Task inspector ended its checking.");
		if (error == true ){
			DISPLAY("Consistency error in the table variable.");
			exit(1);
		}
		xSemaphoreGive ( xSemMutex ) ;
		vTaskDelay(2);
	}
}