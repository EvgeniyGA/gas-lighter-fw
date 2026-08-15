/*
 * meandr_measure.c
 *
 *  Created on: Aug 11, 2026
 *      Author: evgeny
 */
#include "adc_driver.h"
#include "pulse_measure.h"
#include "FreeRTOS.h"
#include "task.h"
#include "limits.h"
#include "string.h"
#include "gpio.h"//todo

typedef enum{
	Pulse_Measure_ADC_Channel_1 = 0,
	Pulse_Measure_ADC_Channel_2,
    Pulse_Measure_ADC_Channel_3,
    Pulse_Measure_ADC_Channel_4,
	Pulse_Measure_ADC_NumbOfCnannels
}pulse_measure_channels_e;

#define PULSE_MEASURE_ADC_DMA_STEPS			(2048)
#define PULSE_MEASURE_ADC_DMA_BUFFER_SIZE 	(Pulse_Measure_ADC_NumbOfCnannels * PULSE_MEASURE_ADC_DMA_STEPS * 2)
#define PULSE_MEASURE_TASK_STACK_SIZE	(configMINIMAL_STACK_SIZE*2)
#define PULSE_MEASURE_INDENT_CYCLES			(100)

uint16_t pulse_measure_adc_dma_buffer[PULSE_MEASURE_ADC_DMA_BUFFER_SIZE];

static TaskHandle_t pulse_measure_task_handle = NULL;
static StaticTask_t pulse_measure_task_def;

void pulse_measure_adc_callback(uint8_t offset){
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	xTaskNotifyFromISR(pulse_measure_task_handle, offset, eSetValueWithOverwrite, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void pulse_measure_task(void* param){
	pulseMeasureConfig_s* pulse_measure_config = (pulseMeasureConfig_s*)param;
	uint32_t offset;
	uint32_t result[Pulse_Measure_ADC_NumbOfCnannels] = {0};
	while(1){
		if (xTaskNotifyWait(0, ULONG_MAX, &offset, portMAX_DELAY) == pdPASS)
		{
			memset(result, 0x00, sizeof(result));
			uint32_t adc_samples_per_ch = pulse_measure_config->buf_adc_in_size /(2 * Pulse_Measure_ADC_NumbOfCnannels);
			offset = offset*pulse_measure_config->buf_adc_in_size/2;
			for (size_t i = PULSE_MEASURE_INDENT_CYCLES*Pulse_Measure_ADC_NumbOfCnannels; 
						i < (pulse_measure_config->buf_adc_in_size/2); 
						i += Pulse_Measure_ADC_NumbOfCnannels){
				for(size_t j = 0; j < Pulse_Measure_ADC_NumbOfCnannels; j++){
					result[j] += pulse_measure_config->buf_adc_in[i + j + offset];
				}
			}
			for(size_t j = 0; j < Pulse_Measure_ADC_NumbOfCnannels; j++){
				result[j] /= (adc_samples_per_ch - PULSE_MEASURE_INDENT_CYCLES);
			}
			if(offset == 0){
				HAL_GPIO_WritePin (en_led2a_GPIO_Port, en_led2a_Pin, GPIO_PIN_SET);
			}
			else{
				HAL_GPIO_WritePin (en_led2a_GPIO_Port, en_led2a_Pin, GPIO_PIN_RESET);
				printf("result %04d:%04d:%04d:%04d\n\r", result[0], result[1], result[2], result[3]);
			}
		}
		//vTaskDelay(100);
	}
}

void pulse_measure_init(pulseMeasureConfig_s* pulse_measure_config){
	static StackType_t  pulse_measure_stack[PULSE_MEASURE_TASK_STACK_SIZE];

	pulse_measure_config->adc_num = ADC_NUM_1;
	pulse_measure_config->buf_adc_in = pulse_measure_adc_dma_buffer;
	pulse_measure_config->buf_adc_in_size = sizeof(pulse_measure_adc_dma_buffer)/sizeof(pulse_measure_adc_dma_buffer[0]);

    adc_driver_start(ADC_NUM_1, pulse_measure_adc_dma_buffer, PULSE_MEASURE_ADC_DMA_BUFFER_SIZE);

	pulse_measure_task_handle = xTaskCreateStatic(pulse_measure_task, "pulse_measure", PULSE_MEASURE_TASK_STACK_SIZE,
			pulse_measure_config, configMAX_PRIORITIES - 3 , pulse_measure_stack, &pulse_measure_task_def);
}
