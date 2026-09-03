/*
 * wave_measure.h
 *
 *  Created on: Apr 5, 2026
 *      Author: evgeny
 */

#ifndef INC_WAVE_MEASURE_H_
#define INC_WAVE_MEASURE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "arm_math.h"

#ifndef MAIN_TIME_RESOLUTION
	#error "MAIN_TIME_RESOLUTION must be defined via CMAKE or compiler flag"
#endif

#define ADC_DMA_STEPS			(MAIN_TIME_RESOLUTION)
#define ADC_DMA_CYCLES			(32)//(64)

typedef struct{
	float32_t main_freq_Hz;
	float32_t main_phase_deg;
}waveMeasureFFT_result_t;

typedef void(*wave_measure_data_ready_t)(waveMeasureFFT_result_t*);

typedef struct{
	uint8_t adc_num;
	uint16_t* buf_adc_in;
	uint16_t buf_adc_in_size;
	float32_t* buf_fft_mag_result;
	uint8_t numb_of_channels;
	void(*adc_callback)(void);
	uint64_t adc_sample_rate;
	uint16_t main_freqency;
	uint16_t time_resolution;
	wave_measure_data_ready_t data_ready;
}waveMeasureConfig_s;

typedef enum{
	WAVE_MEASURE_Channel_1 = 0,
	WAVE_MEASURE_Channel_2,
	WAVE_MEASURE_NumbOfCnannels
}wave_measure_channels_e;

double process_buffer(uint16_t *buffer, uint8_t numb_of_channels);
void wave_measure_adc_callback(uint8_t);
int wave_measure_init(waveMeasureConfig_s* wave_measure_config);

#ifdef __cplusplus
}
#endif

#endif /* INC_WAVE_MEASURE_H_ */
