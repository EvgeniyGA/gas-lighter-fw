/*
 * wave_gen.c
 *
 *  Created on: Mar 30, 2026
 *      Author: evgeny
 */
#include "wave_gen.h"
#include <math.h>

uint16_t calc_sample(waveGenConfig_s* config, float step){
	uint32_t dac_max_dig = pow(2, config->dac_resolution) - 1;
	float dac_digs_in_Volt 	= dac_max_dig / config->dac_reference;
	uint32_t midpoint_dig 	= dac_digs_in_Volt*config->midpoint;
	int delta_dig 			= dac_digs_in_Volt*config->amplitude*config->fun(step);
	return midpoint_dig + delta_dig;
}

int initWaveMas(waveGenConfig_s* config){
	if(!config->numb_of_steps || !config->freq || !config->timer_frequency){
		return -1;
	}

	uint64_t tmp =  (uint64_t)config->timer_frequency / ((uint64_t)config->numb_of_steps * (uint64_t)config->freq);

	if(tmp > 0xFFFF){
		return -1;
	}

	config->timer_arr = tmp;

	if((config->midpoint + config->amplitude > config->dac_reference) ||
	   (config->midpoint < config->amplitude)){
		return -1;
	}

	float step_change = 2 * M_PI / config->numb_of_steps;
	float next_step = 0;
	uint16_t new_val = 0;
	for(int i = 0; i < config->numb_of_steps; i++){
		new_val = calc_sample(config, next_step);
		config->buf[i] = new_val;
		next_step += step_change;
	}
	//if()//not harmonic

	return 0;
}
