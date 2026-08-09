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
#include "app.h"

#define ADC_DMA_STEPS			(MAIN_TIME_RESOLUTION)
#define ADC_DMA_CYCLES			(64)

typedef enum {
	WAVE_MEASURE_OFFSET_ZERO,
	WAVE_MEASURE_OFFSET_HALF
}wave_measure_offset_e;

typedef struct{
	uint16_t* buf_adc_in;
	uint16_t buf_adc_in_size;
	float32_t* buf_fft_mag_result;
	uint8_t numb_of_channels;
	void(*adc_callback)(void);
	uint64_t adc_sample_rate;
	uint16_t main_freqency;
	uint16_t time_resolution;
}waveMeasureConfig_s;


double process_buffer(uint16_t *buffer, uint8_t numb_of_channels);
void wave_measure_adc_callback(void);
int wave_measure_init(waveMeasureConfig_s* wave_measure_config);

#ifdef __cplusplus
}
#endif

#endif /* INC_WAVE_MEASURE_H_ */
