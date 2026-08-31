/*
 * wave_gen.h
 *
 *  Created on: Mar 30, 2026
 *      Author: evgeny
 */

#ifndef INC_WAVE_GEN_H_
#define INC_WAVE_GEN_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DAC_TIMER_FREQENCY_MHZ		(84)

typedef struct{
	float freq;
	float midpoint;
	float amplitude;
	uint16_t dac_max;
	float(*fun)(float);
	uint16_t *buf;
	uint32_t timer_frequency;
	uint16_t dac_resolution;
	uint16_t dac_reference;
	uint16_t numb_of_steps;
	uint16_t timer_arr;
}waveGenConfig_s;

int initWaveMas(waveGenConfig_s* config);

#ifdef __cplusplus
}
#endif

#endif /* INC_WAVE_GEN_H_ */
