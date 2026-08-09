/*
 * wave_runner.c
 *
 *  Created on: Mar 31, 2026
 *      Author: evgeny
 */
#include <wave_starter.h>
#include "dac_driver.h"

uint16_t dac_dma_buff[WAVE_TIME_RESOLUTION];

int wave_starter_init(waveGenConfig_s* config){
	config->amplitude = 1.3;
	config->midpoint = 1.5;
	config->buf = dac_dma_buff;
	//config->numb_of_steps = WAVE_TIME_RESOLUTION;
	config->dac_resolution = 12;
	config->dac_reference = 3;
	config->timer_frequency = DAC_TIMER_FREQENCY_MHZ*1000000;
	return 0;
}

int wave_starter_run(waveGenConfig_s* config){
	if(initWaveMas(config) == 0){
		return dac_start(config->buf, config->numb_of_steps, config->timer_arr);
	}
	return -1;
}

