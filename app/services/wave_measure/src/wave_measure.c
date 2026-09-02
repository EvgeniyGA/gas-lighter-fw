/*
 * wave_measure.c
 *
 *  Created on: Apr 5, 2026
 *      Author: evgeny
 */

#include "wave_measure.h"
#include "math.h"
#include "arm_math.h"
#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "adc_driver.h"

#define VREFINT_CAL_VREF_MV                   ( 3300UL)
#define VREFINT_CAL_ADDR_MV                   ((uint16_t*) (0x1FFF7A2AU))
#define ADC_RESOLUTION pow(2, 12)

typedef enum{
    WAVE_MEASURE_ADC_NUM_1 = 1,
    WAVE_MEASURE_ADC_NUM_2,
    WAVE_MEASURE_ADC_NUM_3
}wave_measure_adc_numbers_e;

#define ADC_DMA_BUFFER_SIZE 	(WAVE_MEASURE_NumbOfCnannels * ADC_DMA_STEPS * ADC_DMA_CYCLES * 2)
#define FFT_BUF_SIZE			(ADC_DMA_STEPS * ADC_DMA_CYCLES)

#define WAVE_MEASURE_TASK_STACK_SIZE	(configMINIMAL_STACK_SIZE*2)

uint16_t adc_dma_buffer[ADC_DMA_BUFFER_SIZE];
float32_t fftBufIn[FFT_BUF_SIZE], fftBufOut[FFT_BUF_SIZE];
float32_t fftBufPhases[FFT_BUF_SIZE/2];
arm_rfft_fast_instance_f32 fftHandler;

static TaskHandle_t wave_measure_task_handle = NULL;
static StaticTask_t wave_measure_task_def;

static uint8_t fft_buffer(waveMeasureConfig_s* wave_measure_config, uint8_t channel, uint8_t offset, waveMeasureFFT_result_t* result);

extern uint8_t adc_driver_start(uint8_t adc_num, uint16_t* buff, uint16_t size);//todo

void wave_measure_adc_callback(uint8_t offset){
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	xTaskNotifyFromISR(wave_measure_task_handle, offset, eSetValueWithOverwrite, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void wave_measure_task(void* param);

void wave_measure_task(void* param){
	waveMeasureConfig_s* wave_measure_config = (waveMeasureConfig_s*)param;
	waveMeasureFFT_result_t result[2]; //todo result_ch1, result_ch2;
	uint32_t offset;

	while(1){
		if (xTaskNotifyWait(0, ULONG_MAX, &offset, portMAX_DELAY) == pdPASS)
		{
			fft_buffer(wave_measure_config, WAVE_MEASURE_Channel_1, offset, &result[WAVE_MEASURE_Channel_1]);
			fft_buffer(wave_measure_config, WAVE_MEASURE_Channel_2, offset, &result[WAVE_MEASURE_Channel_2]);

			wave_measure_config->data_ready(result);

			// lcd_print(2, 0, "%5d Hz, %5d Hz", (int)result_ch1.main_freq_Hz, (int)result_ch2.main_freq_Hz);
			// if(result_ch1.main_freq_Hz == result_ch2.main_freq_Hz){
			// 	float diff = result_ch1.main_phase_deg - result_ch2.main_phase_deg;
			// 	int diff_x100 = (int)(diff * 100);
			// 	lcd_print(3, 0, "dPhase: %d.%02d deg", diff_x100/100, abs(diff_x100%100));//todo
			// }
			// else{
			// 	lcd_print(2, 0, "                    ");
			// }
		}
		vTaskDelay(1000);
	}
}

int wave_measure_init(waveMeasureConfig_s* wave_measure_config){
	static StackType_t  wave_measure_stack[WAVE_MEASURE_TASK_STACK_SIZE];
	memset(adc_dma_buffer, 0x00, sizeof(adc_dma_buffer[0])*ADC_DMA_BUFFER_SIZE);
	wave_measure_config->buf_adc_in = adc_dma_buffer;
	wave_measure_config->buf_adc_in_size = sizeof(adc_dma_buffer)/sizeof(adc_dma_buffer[0]);
	wave_measure_config->adc_num = WAVE_MEASURE_ADC_NUM_2;
	wave_measure_config->numb_of_channels = WAVE_MEASURE_NumbOfCnannels;
	wave_measure_config->adc_sample_rate = wave_measure_config->main_freqency*
			wave_measure_config->time_resolution/wave_measure_config->numb_of_channels;

	arm_rfft_fast_init_f32(&fftHandler, FFT_BUF_SIZE );
	adc_driver_register_callback(ADC_NUM_2, wave_measure_adc_callback);
	adc_driver_start(wave_measure_config->adc_num, adc_dma_buffer, ADC_DMA_BUFFER_SIZE);
	wave_measure_task_handle = xTaskCreateStatic(wave_measure_task, "wave_measure", WAVE_MEASURE_TASK_STACK_SIZE,
			wave_measure_config, configMAX_PRIORITIES - 3 , wave_measure_stack, &wave_measure_task_def);
	return 0;
}


uint8_t fft_buffer(waveMeasureConfig_s* wave_measure_config, uint8_t channel, uint8_t offset, waveMeasureFFT_result_t* result){
	float32_t freq = 0, main_freq = 0, tmp_max = 0;
	uint16_t main_bin = 0;
	uint16_t offset_ = offset*wave_measure_config->buf_adc_in_size/2;
	if(channel >= WAVE_MEASURE_NumbOfCnannels){
		return -1;
	}
	for(int i = 0; i < FFT_BUF_SIZE; i++){
		fftBufIn[i] = (float32_t)wave_measure_config->buf_adc_in[i*2 + channel + offset_];
	}
	arm_rfft_fast_f32(&fftHandler, fftBufIn, fftBufOut, 0);

	for(int i = 1; i < FFT_BUF_SIZE/2; i++){
		float32_t re = fftBufOut[2*i];
		float32_t im = fftBufOut[2*i + 1];
		fftBufPhases[i] = atan2f(im, re);
	}

	arm_cmplx_mag_f32(fftBufOut, fftBufOut, FFT_BUF_SIZE/2);
	for(int i = 1; i < FFT_BUF_SIZE/2; i++){
		freq = (float32_t)(i * wave_measure_config->adc_sample_rate * 2) / FFT_BUF_SIZE;
		if(tmp_max < fftBufOut[i]){
			tmp_max = fftBufOut[i];
			main_freq = freq;
			main_bin = i;
		}
	}

	result->main_phase_deg = fftBufPhases[main_bin]*180.0f / M_PI;
	int neibour = (main_bin > 1) ? (main_bin - 1) : (main_bin + 1);
	if(fftBufOut[neibour]*100 < fftBufOut[main_bin]){
		result->main_freq_Hz = main_freq / wave_starter_get_divider();
		return 0;
	}
	else{
		result->main_freq_Hz = 0;
		return -1;
	}
}

