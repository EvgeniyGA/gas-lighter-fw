/*
 * meandr_measure.c
 *
 *  Created on: Aug 11, 2026
 *      Author: evgeny
 */
#include <stdio.h>
#include "adc_driver.h"
#include "pulse_measure.h"
#include "FreeRTOS.h"
#include "task.h"
#include "limits.h"
#include "string.h"
#include "gpio.h"//todo

#ifndef FOR_QEMU
#define PULSE_MEASURE_ADC_DMA_STEPS			(2048)
#else
#define PULSE_MEASURE_ADC_DMA_STEPS			(2)
#endif

#define PULSE_MEASURE_ADC_DMA_BUFFER_SIZE 	(Pulse_Measure_ADC_NumbOfCnannels * PULSE_MEASURE_ADC_DMA_STEPS * 2)
#define PULSE_MEASURE_TASK_STACK_SIZE		(configMINIMAL_STACK_SIZE*2)
#define PULSE_MEASURE_INDENT_CYCLES			(100)

uint16_t pulse_measure_adc_dma_buffer[PULSE_MEASURE_ADC_DMA_BUFFER_SIZE];

static TaskHandle_t pulse_measure_task_handle = NULL;
static StaticTask_t pulse_measure_task_def;
static uint32_t measure_divider = 1;

void pulse_measure_adc_callback(uint8_t offset){
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	xTaskNotifyFromISR(pulse_measure_task_handle, offset, eSetValueWithOverwrite, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void pulse_measure_task(void* param){
	pulseMeasureConfig_s* pulse_measure_config = (pulseMeasureConfig_s*)param;
	uint32_t offset;
	uint32_t result[Pulse_Measure_ADC_NumbOfCnannels] = {0};
	pulse_measure_msg_t data_msg;
	adc_driver_register_callback(ADC_NUM_1, pulse_measure_adc_callback);
	vTaskDelay(5000 / portTICK_PERIOD_MS);
	while(1){
		if (xTaskNotifyWait(0, ULONG_MAX, &offset, portMAX_DELAY) == pdPASS)
		{
			memset(result, 0x00, sizeof(result));
			uint32_t adc_samples_per_ch = pulse_measure_config->buf_adc_in_size / measure_divider / (2 * Pulse_Measure_ADC_NumbOfCnannels);
			offset = offset*pulse_measure_config->buf_adc_in_size/2;
			for (size_t i = PULSE_MEASURE_INDENT_CYCLES*Pulse_Measure_ADC_NumbOfCnannels; 
						i < (pulse_measure_config->buf_adc_in_size/(2 * measure_divider)); 
						i += Pulse_Measure_ADC_NumbOfCnannels){
				for(size_t j = 0; j < Pulse_Measure_ADC_NumbOfCnannels; j++){
					result[j] += pulse_measure_config->buf_adc_in[i + j + offset];
				}
			}
			for(size_t j = 0; j < Pulse_Measure_ADC_NumbOfCnannels; j++){
				result[j] /= (adc_samples_per_ch - PULSE_MEASURE_INDENT_CYCLES);
			}
			if(offset == 0){
				//HAL_GPIO_WritePin (en_led2a_GPIO_Port, en_led2a_Pin, GPIO_PIN_SET);
				pulse_measure_config->event_full();
			}
			else{
				//HAL_GPIO_WritePin (en_led2a_GPIO_Port, en_led2a_Pin, GPIO_PIN_RESET);
				pulse_measure_config->event_half();
				for(int i = 0; i < Pulse_Measure_ADC_NumbOfCnannels; i++){
					data_msg.result[i] = result[i];
				}
				pulse_measure_config->data_ready(&data_msg);
			}
		}
	}
}

void pulse_measure_init(pulseMeasureConfig_s* pulse_measure_config){
	static StackType_t  pulse_measure_stack[PULSE_MEASURE_TASK_STACK_SIZE];

	pulse_measure_config->adc_num = ADC_NUM_1;
	pulse_measure_config->buf_adc_in = pulse_measure_adc_dma_buffer;
	pulse_measure_config->buf_adc_in_size = sizeof(pulse_measure_adc_dma_buffer)/sizeof(pulse_measure_adc_dma_buffer[0]);

    adc_driver_start(ADC_NUM_1, pulse_measure_adc_dma_buffer, PULSE_MEASURE_ADC_DMA_BUFFER_SIZE/measure_divider);

	pulse_measure_task_handle = xTaskCreateStatic(pulse_measure_task, "pulse_measure", PULSE_MEASURE_TASK_STACK_SIZE,
			pulse_measure_config, configMAX_PRIORITIES - 3 , pulse_measure_stack, &pulse_measure_task_def);
}

uint8_t puse_measure_set_divider(uint32_t divider){
	if ( (PULSE_MEASURE_ADC_DMA_BUFFER_SIZE/divider) % (Pulse_Measure_ADC_NumbOfCnannels * 2) != 0){
		return -1;
	}
	measure_divider = divider;
	adc_driver_stop(ADC_NUM_1);
	adc_driver_start(ADC_NUM_1, pulse_measure_adc_dma_buffer, PULSE_MEASURE_ADC_DMA_BUFFER_SIZE/measure_divider);
	return 0;
}