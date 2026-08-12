/*
 * wave_measure.c
 *
 *  Created on: Apr 5, 2026
 *      Author: evgeny
 */

#include "wave_measure.h"
#include "process_adc.h"
#include "adc.h"
#include "adc_driver.h"
#include "math.h"
#include "arm_math.h"
#include "FreeRTOS.h"
#include "task.h"

#define VREFINT_CAL_VREF_MV                   ( 3300UL)
#define VREFINT_CAL_ADDR_MV                   ((uint16_t*) (0x1FFF7A2AU))
#define ADC_RESOLUTION pow(2, 12)

typedef enum{
	ADC_Channel_1,
	ADC_Channel_2,
	ADC_NumbOfCnannels
}wave_measure_channels_e;

typedef struct{
	float32_t main_freq_Hz;
	float32_t main_phase_deg;
}waveMeasureFFT_result;

#define ADC_DMA_BUFFER_SIZE 	(ADC_NumbOfCnannels * ADC_DMA_STEPS * ADC_DMA_CYCLES * 2)
#define FFT_BUF_SIZE			(ADC_DMA_STEPS * ADC_DMA_CYCLES)

#define WAVE_MEASURE_TASK_STACK_SIZE	(configMINIMAL_STACK_SIZE*2)

uint16_t adc_dma_buffer[ADC_DMA_BUFFER_SIZE];
float32_t fftBufIn[FFT_BUF_SIZE], fftBufOut[FFT_BUF_SIZE];
float32_t fftBufPhases[FFT_BUF_SIZE/2];
arm_rfft_fast_instance_f32 fftHandler;

static TaskHandle_t wave_measure_task_handle = NULL;
static StaticTask_t wave_measure_task_def;

static uint8_t fft_buffer(waveMeasureConfig_s* wave_measure_config, uint8_t channel, uint8_t offset, waveMeasureFFT_result* result);

void wave_measure_adc_callback(uint8_t offset){
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	xTaskNotifyFromISR(wave_measure_task_handle, offset, eSetValueWithOverwrite, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void wave_measure_task(void* param);

void wave_measure_task(void* param){
	waveMeasureConfig_s* wave_measure_config = (waveMeasureConfig_s*)param;
	waveMeasureFFT_result result_ch1, result_ch2;
	uint32_t offset;

	while(1){
		if (xTaskNotifyWait(0, ULONG_MAX, &offset, portMAX_DELAY) == pdPASS)
		{
			fft_buffer(wave_measure_config, ADC_Channel_1, offset, &result_ch1);
			fft_buffer(wave_measure_config, ADC_Channel_2, offset, &result_ch2);

			printf("F1: %6.2f Hz, F2: %6.2f Hz\t", result_ch1.main_freq_Hz, result_ch2.main_freq_Hz);
			if(result_ch1.main_freq_Hz == result_ch2.main_freq_Hz){
				printf("delta Phase, deg: %3.2f\n\r", result_ch1.main_phase_deg - result_ch2.main_phase_deg);
			}
			else{
				printf("\n\r");
			}
		}
		vTaskDelay(1000);
	}
}

int wave_measure_init(waveMeasureConfig_s* wave_measure_config){
	static StackType_t  wave_measure_stack[WAVE_MEASURE_TASK_STACK_SIZE];
	memset(adc_dma_buffer, 0x00, sizeof(adc_dma_buffer[0])*ADC_DMA_BUFFER_SIZE);
	wave_measure_config->buf_adc_in = adc_dma_buffer;
	wave_measure_config->buf_adc_in_size = sizeof(adc_dma_buffer)/sizeof(adc_dma_buffer[0]);
	wave_measure_config->adc_num = ADC_NUM_2;
	wave_measure_config->numb_of_channels = ADC_NumbOfCnannels;
	//wave_measure_config->adc_callback = wave_measure_adc_callback;
	wave_measure_config->adc_sample_rate = wave_measure_config->main_freqency*
			wave_measure_config->time_resolution/wave_measure_config->numb_of_channels;

	arm_rfft_fast_init_f32(&fftHandler, FFT_BUF_SIZE );
	adc_driver_start(wave_measure_config->adc_num, adc_dma_buffer, ADC_DMA_BUFFER_SIZE);
	wave_measure_task_handle = xTaskCreateStatic(wave_measure_task, "wave_measure", WAVE_MEASURE_TASK_STACK_SIZE,
			wave_measure_config, configMAX_PRIORITIES - 3 , wave_measure_stack, &wave_measure_task_def);
	return 0;
}


uint8_t fft_buffer(waveMeasureConfig_s* wave_measure_config, uint8_t channel, uint8_t offset, waveMeasureFFT_result* result){
	float32_t freq = 0, main_freq = 0, tmp_max = 0;
	uint16_t main_bin = 0;
	uint16_t offset_ = offset*wave_measure_config->buf_adc_in_size/2;
	if(channel >= ADC_NumbOfCnannels){
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
		result->main_freq_Hz = main_freq;
		return 0;
	}
	else{
		result->main_freq_Hz = 0;
		return -1;
	}
}

/*double process_buffer(uint16_t *buffer, uint8_t numb_of_channels) {
	double phot1 = 0.0;
	double phot2 = 0.0;
    uint32_t phot1_sum = 0, phot2_sum = 0;
    uint16_t phot1_avg, phot2_avg;

    for (int i = 0; i < ADC_DMA_SAMPLES; ++i) {
    	phot1_sum += buffer[0];
    	phot2_sum += buffer[1];
        buffer += 2;
    }

    phot1_avg = phot1_sum / ADC_DMA_SAMPLES;
    phot2_avg = phot2_sum / ADC_DMA_SAMPLES;

    phot1 = (float) VREFINT_CAL_VREF_MV * phot1_avg / ADC_RESOLUTION / 1000;
    phot2 = (float) VREFINT_CAL_VREF_MV * phot2_avg / ADC_RESOLUTION / 1000;

    usbTxBufLen2 = sprintf((char*)usbTxBuf2,"ADC: %1u\r\n",  HAL_GetTick());
    CDC_Transmit_FS(usbTxBuf2, usbTxBufLen2);
    return phot2;
}*/
