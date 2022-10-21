/****************************************************************************
 * Copyright (C) 2020 by Fabrice Muller                                     *
 *                                                                          *
 * This file is useful for ESP32 Design course.                             *
 *                                                                          *
 ****************************************************************************/

/**
 * @file lab2-4_main.c
 * @author Fabrice Muller
 * @date 13 Oct. 2020
 * @brief File containing the lab2-4 of Part 3.
 *
 * @see https://github.com/fmuller-pns/esp32-vscode-project-template
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "esp_log.h"
#include "soc/rtc_wdt.h"
#include "driver/gpio.h"

#include "freertos/FreeRTOSConfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "my_helper_fct.h"

static const char* TAG = "MsgQ";

// Push button for interrupt
static const gpio_num_t PIN_PUSH_BUTTON = 15;
uint32_t isrCount = 0ULL;

/* Stack size for all tasks */
const uint32_t TASK_STACK_SIZE = 4096;
static const uint32_t T1_PRIO = 5;
QueueHandle_t xQueue = NULL;

void vCounterTask(void *pvParameter );
static void IRAM_ATTR Push_button_isr_handler(void *args);


void app_main(void) {

	/* Config GPIO */
	gpio_config_t config_in;
	config_in.intr_type = GPIO_INTR_NEGEDGE ;			//Falling edge interrupt 
	config_in.mode = GPIO_MODE_INPUT;
	config_in.pull_down_en = false;
	config_in.pull_up_en = true ;						// pull up resistor
	config_in.pin_bit_mask = (1ULL<<PIN_PUSH_BUTTON);	//pin number
	gpio_config(&config_in);

	/* Create Message Queue and Check if created */
	xQueue =xQueueCreate(5, sizeof(uint32_t));
	if( xQueue == 0 ) { DISPLAY("Failed ")};	

	/* Create vCounterTask task */
	xTaskCreate(vCounterTask, "vCounterTask", TASK_STACK_SIZE, (void*)"vCounterTask", T1_PRIO,NULL);

	/* Install ISR */
	gpio_install_isr_service(0);
	gpio_isr_handler_add(PIN_PUSH_BUTTON, Push_button_isr_handler, (void*) PIN_PUSH_BUTTON);


	/* to ensure its exit is clean */
	vTaskDelete(NULL);
}

void vCounterTask(void *pvParameters) {

	char *pcTaskName;
	pcTaskName = (char *)pvParameters;
	uint32_t RecievedValue;
	DISPLAY("Start of %s task, priority = %d",pcTaskName, uxTaskPriorityGet(NULL));
	
	UBaseType_t NbItems;

	for (;; ) {

		/* Wait for message with 5 sec. otherwise DISPLAY a message to push it */
		
		/* If pushed */
		if(xQueueReceive( xQueue, &RecievedValue,pdMS_TO_TICKS(5000))){//pdMS_TO_TICKS(300) // portMAX_DELAY
		
			DISPLAYI(TAG, "Task %s,Button pushed, recieved value = %d",pcTaskName,RecievedValue);
			
			// Get the number of items in the queue
			NbItems=uxQueueMessagesWaiting(xQueue );

			// DISPLAYI (Information display) number of items if greater than 1
			if(NbItems>1){
				DISPLAYI(TAG, "number of items in the queue : %x",NbItems ); 
			}

			// Waiting for push button signal is 1 again (test every 20 ms)
			while(1){
					COMPUTE_IN_TICK(2);
					if(gpio_get_level(PIN_PUSH_BUTTON))
					break;
				}
			// DISPLAY "Button released"
			DISPLAY("Button released" );

		/* Else If Time out */
		}else {
			// Display message "Please, push the button!"
			DISPLAYE ( TAG , "  Timeout ! Push the button!! " ) ;
			COMPUTE_IN_TICK (1) ;
			
		}

	}
}


static void IRAM_ATTR Push_button_isr_handler(void *args) {

	// Increment isrCount
	isrCount++;
	// Send message
	xQueueSendFromISR(xQueue, &isrCount, NULL);

}

